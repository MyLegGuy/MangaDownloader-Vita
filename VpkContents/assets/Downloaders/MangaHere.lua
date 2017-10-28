-- Started 10/27/17


SCRIPTVERSION=1;
SAVEVARIABLE=0;

currentMangaUrlList = {};
currentMangaNameList = {};

--

function onListMoreInfo(listId, listEntry)
	print("List: " .. listId)
	print("Entry: " .. listEntry)
end

-- TODO - Make this
function getChapterList(_passedUrl)
	local _returnTable
	showStatus("Getting chapter list HTML...");
	local _tempDownloadedHTML = downloadString(_passedUrl);
	showStatus("Parsing chapter list HTML...");
	local _firstFound;
	_firstFound = string.find(_tempDownloadedHTML,"detail_list",1,true);
	while (true) do

	end
end

-- Fix series urls (and chapter urls?)
function fixUrl(_passedUrl)
	return ("https://www." .. _passedUrl);
end

function getMangaList()
	showStatus("Getting manga list HTML...");
	local _tempDownloadedHTML = downloadString("https://www.mangahere.co/mangalist/");
	showStatus("Parsing manga list HTML...");
	local _firstFound;
	local _secondFound;
	_firstFound = string.find(_tempDownloadedHTML,"list_manga",0,true);
	while (true) do
		--manga_info
		--<li><a class="manga_info" rel="&quot;Aishiteru&quot;, Uso Dakedo." href="//www.mangahere.co/manga/aishiteru_uso_dakedo/"><span></span>&quot;Aishiteru&quot;, Uso Dakedo.</a></li>
		_firstFound = string.find(_tempDownloadedHTML,"manga_info",_firstFound+1,true);
		if (_firstFound==nil) then
			break;
		end
		_firstFound = string.find(_tempDownloadedHTML,"href=",_firstFound,true);
		-- Add 12 to this result. This is to get past href=, the quotation mark, the two random slashes at the start of the url, and www.
		_firstFound = _firstFound+12;
		_secondFound = string.find(_tempDownloadedHTML,"\"",_firstFound,true);
		table.insert(currentMangaUrlList,string.sub(_tempDownloadedHTML,_firstFound,_secondFound-1));
		-- Name of manga starts after useless span tags.
		_firstFound = string.find(_tempDownloadedHTML,"</span>",_firstFound,true);
		_firstFound = _firstFound+7; -- Offset past </span>
		_secondFound = string.find(_tempDownloadedHTML,"</a>",_firstFound,true);
		table.insert(currentMangaNameList,fixHtmlStupidity(string.sub(_tempDownloadedHTML,_firstFound,_secondFound-1)));
	end
end

function InitList01(isFirstTime)
	if (isFirstTime>=1) then
		return currentMangaNameList;
	else
		return nil;
	end
end

function EndList01()
	--getChapterList(seriesListUrl[userInput01])
	--if (#chapterListFriendly>0) then
	--	assignListData(currentQueueCLists,currentQueueCListsLength,1,chapterListFriendly)
	--end
end

function MyLegGuy_Download()
	-- userInput01
	print("Download here.")
end

function getWebpagesImage(url, _passedStatusPrefix)
	-- Relevant section
	--[[
		<section class="read_img" id="viewer">
		<a href="//www.mangahere.co/manga/15_sai_asagi_ryuu/c001/29.html" onclick="return next_page();">
				<img id="loading" class="loadingImg" src="//www.mangahere.co/media/images/loading.gif" />
			<img src="https://mhcdn.secure.footprint.net/store/manga/17947/001.0/compressed/vimg028.jpg?token=f17a6e6a59029591129882a40dae1b8fdd953fb7&ttl=1509246000" onload="loadImg(this)" style="display: none;margin:0 auto;" width="869" id="image" alt="15-sai (ASAGI Ryuu) 1 Page 28" />
		</a>
		
		<div id="tsuk_container" class="tsuk-info" style="display:none;">
			<div class="info-content"></div>
		</div>
		</section>
	]]
	showStatus(_passedStatusPrefix .. "Getting webpage HTML...");
	local _tempDownloadedHTML = downloadString(url);
	showStatus(_passedStatusPrefix .. "Parsing webpage HTML...");
	local _firstFound = string.find(_tempDownloadedHTML,"read_img",1,true);
	_firstFound = string.find(_tempDownloadedHTML,"src=",_firstFound,true); -- This is the image source for the loading gif
	_firstFound = string.find(_tempDownloadedHTML,"src=",_firstFound+1,true); -- This is the image source for the actual manga page
	_firstFound = _firstFound+5;
	_secondFound = string.find(_tempDownloadedHTML,"\"",_firstFound+1,true);
	print((string.sub(_tempDownloadedHTML,_firstFound,_secondFound-1)))
end

function MyLegGuy_Prompt()
	disableSSLVerification();
	
	getMangaList();
	--happy = loadImageFromUrl("https://www.google.com/images/branding/googlelogo/1x/googlelogo_color_272x92dp.png",FILETYPE_PNG)
	--photoViewer(happy)
	--freeTexture(happy)
	
	--downloadFile("https://www.mangahere.co/mangalist/","./testdownloaded4")

	userInputQueue("Manga","Choose the manga series you want.",INPUTTYPELIST)
	waitForUserInputs();
end