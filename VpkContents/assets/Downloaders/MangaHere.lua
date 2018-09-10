-- Started 10/27/17. Done 10/28/17

SCRIPTVERSION=1;
SAVEVARIABLE=0;

currentMangaUrlList = {};
currentMangaNameList = {};

currentMangaChapterUrlList = {};
currentMangaChapterNameList = {};

--

function onListMoreInfo(listId, listEntry)
	print("List: " .. listId)
	print("Entry: " .. listEntry)
end

function trimString(s)
  return (s:gsub("^%s*(.-)%s*$", "%1"))
end

function reverseTable(_inTable)
	for i=1,math.floor(#_inTable/2) do
		local _otherIndex = #_inTable-(i-1);
		local _cachedInput = _inTable[_otherIndex];
		_inTable[_otherIndex]=_inTable[i];
		_inTable[i]=_cachedInput;
	end
	return _inTable;
end

function getChapterList(_passedUrl)
	--[[
	<div class="detail_list">
        <div class="title"><span></span><h3>Read 15-sai (ASAGI Ryuu) Online</h3></div>
	<ul><li>
           <span class="left">
        <a class="color_0077" href="//www.mangahere.cc/manga/15_sai_asagi_ryuu/c005/">
          15-sai (ASAGI Ryuu) 5            </a>
      <span class="mr6"></span></span>
                <span class="right">Apr 26, 2016</span>
    </li>]]
	local _returnTableUrl = {};
	local _returnTableName = {};
	showStatus("Getting chapter list HTML...");
	local _tempDownloadedHTML = downloadString(_passedUrl);
	showStatus("Parsing chapter list HTML...");
	local _firstFound;
	local _secondFound;
	_firstFound = string.find(_tempDownloadedHTML,"detail_list",1,true);
	while (true) do
		-- get href url and then between > and <a/>
		_firstFound = string.find(_tempDownloadedHTML,"href=",_firstFound+1,true);
		_firstFound = _firstFound+12;
		_secondFound = string.find(_tempDownloadedHTML,"\"",_firstFound+1,true);
		local _foundUrl = string.sub(_tempDownloadedHTML,_firstFound,_secondFound-1);
		if (string.sub(_foundUrl,1,1)=="/") then
			break;
		end
		table.insert(_returnTableUrl,_foundUrl);
		
		_firstFound = string.find(_tempDownloadedHTML,">",_firstFound+1,true);
		_secondFound = string.find(_tempDownloadedHTML,"</a>",_firstFound+1,true);
		table.insert(_returnTableName,trimString(string.sub(_tempDownloadedHTML,_firstFound+1,_secondFound-1)))
	end
	-- Everything was found in reverse order of release
	_returnTableName = reverseTable(_returnTableName);
	_returnTableUrl = reverseTable(_returnTableUrl);
	return _returnTableName, _returnTableUrl
end


-- Fix series urls and chapter urls. Needs to be used before passing as argument
function fixUrl(_passedUrl)
	return ("https://www." .. _passedUrl);
end

function getMangaList()
	showStatus("Getting manga list HTML...");
	local _tempDownloadedHTML = downloadString("https://www.mangahere.cc/mangalist/");
	showStatus("Parsing manga list HTML...");
	local _firstFound;
	local _secondFound;
	_firstFound = string.find(_tempDownloadedHTML,"list_manga",0,true);
	while (true) do
		--manga_info
		--<li><a class="manga_info" rel="&quot;Aishiteru&quot;, Uso Dakedo." href="//www.mangahere.cc/manga/aishiteru_uso_dakedo/"><span></span>&quot;Aishiteru&quot;, Uso Dakedo.</a></li>
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
	currentMangaChapterNameList, currentMangaChapterUrlList = getChapterList(fixUrl(currentMangaUrlList[userInput01]))
	if (#currentMangaChapterNameList>0) then
		assignListData(currentQueueCLists,currentQueueCListsLength,1,currentMangaChapterNameList)
	end
end

--wid60
function getWebpagesImageRaw(_tempDownloadedHTML)
	-- Relevant section
	--[[
		<section class="read_img" id="viewer">
		<a href="//www.mangahere.cc/manga/15_sai_asagi_ryuu/c001/29.html" onclick="return next_page();">
				<img id="loading" class="loadingImg" src="//www.mangahere.cc/media/images/loading.gif" />
			<img src="https://mhcdn.secure.footprint.net/store/manga/17947/001.0/compressed/vimg028.jpg?token=f17a6e6a59029591129882a40dae1b8fdd953fb7&ttl=1509246000" onload="loadImg(this)" style="display: none;margin:0 auto;" width="869" id="image" alt="15-sai (ASAGI Ryuu) 1 Page 28" />
		</a>
		
		<div id="tsuk_container" class="tsuk-info" style="display:none;">
			<div class="info-content"></div>
		</div>
		</section>
	]]
	local _firstFound = string.find(_tempDownloadedHTML,"read_img",1,true);
	_firstFound = string.find(_tempDownloadedHTML,"src=",_firstFound,true); -- This is the image source for the loading gif
	_firstFound = string.find(_tempDownloadedHTML,"src=",_firstFound+1,true); -- This is the image source for the actual manga page
	_firstFound = _firstFound+5;
	_secondFound = string.find(_tempDownloadedHTML,"\"",_firstFound+1,true);
	return (string.sub(_tempDownloadedHTML,_firstFound,_secondFound-1)); 
end

function getChapterTotalPages(_tempDownloadedHTML)
	--[[
	<select class="wid60" onchange="change_page(this)">
	<option value="//www.mangahere.cc/manga/15_sai_asagi_ryuu/c001/" >1</option>
	<option value="//www.mangahere.cc/manga/15_sai_asagi_ryuu/c001/2.html" >2</option>
	...
	</select>
	<a href="//www.mangahere.cc/manga/15_sai_asagi_ryuu/c001/29.html" class="next_page"></a>]]
	local _firstFound = string.find(_tempDownloadedHTML,"wid60",1,true);
	if (_firstFound==nil) then
		if (string.find(_tempDownloadedHTML,"has been licensed. It's not available in MangaHere",1,true)~=nil) then
			popupMessage("The series has been licensed and isn't available on MangaHere. Sorry.");
			return nil;
		end
	end
	local _endIndex = string.find(_tempDownloadedHTML,"href=",_firstFound+1,true);
	local i=0;
	while (true) do
		_firstFound = string.find(_tempDownloadedHTML,"<option",_firstFound+1,true);
		if (_firstFound==nil or _firstFound>_endIndex) then
			break;
		end
		i=i+1;
	end
	-- 11/6/17 - Return i-1 because they added advertisements at the end.
	return i-1;
end

function getMangaFilepaths(_seriesUrl, _chapterName)
	local _returnMangaSpecificFolder = getMangaFolder(true) .. string.match(_seriesUrl,".*/(.*)/");
	local _returnMangaChapterFolder = _returnMangaSpecificFolder .. "/" .. makeFolderFriendly(_chapterName) .. "/";
	return _returnMangaSpecificFolder, _returnMangaChapterFolder;
end

--	Look for the <manganame> manga string.
-- For example, 15-SAI (ASAGI RYUU) Manga
function doChapter(_passedChapterUrl, _passedStatusPrefix, _passedDownloadDirectory)
	goodShowStatus(_passedStatusPrefix .. "Getting page HTML 1/?");
	-- _passedChapterUrl is actually link to first page
	local _currentDownloadedHTML = downloadString(_passedChapterUrl);
	goodShowStatus(_passedStatusPrefix .. "Parsing number of pages...");
	local _totalPagesToDownload = getChapterTotalPages(_currentDownloadedHTML);
	if (_totalPagesToDownload==nil) then
		return nil;
	end
	for i=1,_totalPagesToDownload do
		-- We already downloaded the first page's HTML
		if (i~=1) then
			goodShowStatus(_passedStatusPrefix .. "Getting page HTML " .. i .. "/" .. _totalPagesToDownload)
			_currentDownloadedHTML = downloadString(_passedChapterUrl .. i .. ".html");
		end
		local _imageUrl = getWebpagesImageRaw(_currentDownloadedHTML);
		goodShowStatus(_passedStatusPrefix .. "Downloading page " .. i .. "/" .. _totalPagesToDownload)
		downloadFile(_imageUrl,_passedDownloadDirectory .. string.format("%03d",i) .. ".jpg")
		goodJustDownloaded()
	end
	return 1;
end

function doChapterBroad(_seriesUrl, _chapterUrl, _chapterName, _statusPrefix)
	local _mangaSpecificFolder, _mangaChapterFolder = getMangaFilepaths(_seriesUrl,_chapterName)
	
	createDirectory(_mangaSpecificFolder);
	createDirectory(_mangaChapterFolder);

	--makeFolderFriendly
	return doChapter(fixUrl(_chapterUrl),_statusPrefix,_mangaChapterFolder);
end

function InitList02(isFirstTime)
	if (isFirstTime>=1) then
		return currentMangaChapterNameList;
	end
	return nil;
end
function MyLegGuy_Download()
	-- userInput01
	print("Download here.")
	
	for i=1,#userInput02 do
		if (doChapterBroad(_currentSeriesUrl,currentMangaChapterUrlList[userInput02[i]],currentMangaChapterNameList[userInput02[i]], i .. "/" .. #userInput02 .. "\n")==nil) then
			break;
		end
	end
	setDoneDownloading()
end
function MyLegGuy_Prompt()
	disableSSLVerification();
	
	--local name, url = getChapterList("www.mangahere.cc/manga/15_sai_asagi_ryuu/")
	--for i=1,#name do
	--	print(name[i])
	--	print(url[i])
	--end
	--doChapterBroad("www.mangahere.cc/manga/15_sai_asagi_ryuu/",url[2],name[2],"test good stuff\n");
	
	--http://www.mangahere.cc/manga/15_sai_asagi_ryuu/c001/3.html
	getMangaList();
	--happy = loadImageFromUrl("https://www.google.com/images/branding/googlelogo/1x/googlelogo_color_272x92dp.png",FILETYPE_PNG)
	--photoViewer(happy)
	--freeTexture(happy)
	
	--downloadFile("https://www.mangahere.cc/mangalist/","./testdownloaded4")
	
	userInputQueue("Manga","Choose the manga series you want.",INPUTTYPELIST)
	if (isAsIGo==false) then
		userInputQueue("Chapter","Choose the chapter of the manga you want.",INPUTTYPELISTMULTI)
	else
		userInputQueue("Chapter","Choose the chapter of the manga you want.",INPUTTYPELIST)
	end
	waitForUserInputs();


	_currentSeriesName = currentMangaNameList[userInput01];
	_currentSeriesUrl = currentMangaUrlList[userInput01];

	

	if (isAsIGo==true) then
		local _currentChapterName = currentMangaChapterNameList[userInput02];
		local _currentChapterUrl = currentMangaChapterUrlList[userInput02];
		_,_asIgoFolder=getMangaFilepaths(_currentSeriesUrl,_currentChapterName);
	end
	-- Change this into an array to work like multi
	if (isAsIGo==true) then
		local _tempCacheVariable = userInput02;
		userInput02 = {};
		table.insert(userInput02,_tempCacheVariable);
	end

	currentMangaUrlList = nil; -- These huge lists aren't needed anymore.
	currentMangaNameList = nil;
end