-- one page
--https://www.webtoons.com/search?keyword=Merryweatherey&searchType=CHALLENGE
-- multiple pages
--https://www.webtoons.com/search?keyword=slice%20of%20life&searchType=CHALLENGE

APPENDPAGEFORMAT = "&page=%d"
SEARCHFORMATURL = ("https://www.webtoons.com/search?keyword=%s" .. APPENDPAGEFORMAT)
-- Applied to fake comic URLs
URLPREFIX = "https://www.webtoons.com"

SCRIPTVERSION=1;
SAVEVARIABLE=0;

currentInputScreen=0;

numSearchResultPages=0;
searchResultUrls={};
searchResultNames={};

lastSearchTerms="";

-- Set after first selection
selectedComicUrl="";
selectedComicName="";

function downloadChapter(chapterUrl)
	goodShowStatus("Getting page URLs");

	_chapterDest = (getMangaFolder(true) .. "testFolder")
	_pageFormat = (_chapterDest .. "/%03d.jpg");
	createDirectory(_chapterDest);

	_pageData = downloadString(chapterUrl);
	_pageTableStart = string.find(_pageData,"_viewerBox",0,true);
	_pageTableEnd = string.find(_pageData,"_bottomEpisodeList",_pageTableStart,true); -- If we find an image url and it's past here, stop.

	_pageUrlTable = {};

	_lastFoundStart=_pageTableStart;
	i=1;
	while (true) do
		_lastFoundStart = string.find(_pageData,"data-url",_lastFoundStart+1,true);
		if (_lastFoundStart==nil or _lastFoundStart>_pageTableEnd) then
			break;
		end
		_foundUrlEnd = string.find(_pageData,"\"",_lastFoundStart+10,true);
		_pageUrlTable[i] = (string.sub(_pageData,_lastFoundStart+10,_foundUrlEnd-1));
		i=i+1;
	end
	i=i-1; -- 'i' now represents the final index

	for j=1,i do
		goodShowStatus("Downloading " .. j "/" .. i);
		print(_pageUrlTable[j])
		downloadFile(_pageUrlTable[j],string.format(_pageFormat,j));
	end

	--os.execute("")
end


--downloadChapter("https://www.webtoons.com/en/challenge/crawling-dreams/episode-1-nyarla-ghast/viewer?title_no=141539&episode_no=1");
--[[
<!-- challenge result -->
<h3 class="search_result">Discover (14 results)</h3>
]]

-- Given the HTML for a page of search results or a comic's main page, return the total number of search result pages. Can return 1 if there's only one page
function getTotalPages(_searchResultHTML)
	_pageListStart = string.find(_searchResultHTML,"paginate",0,true);
	if (_pageListStart==nil) then -- One page of results only
		return 1;
	else -- Multiple pages, find out how many
		_pageListEnd = string.find(_searchResultHTML,"div",_pageListStart,true);
		_foundTotalResultPages=0;
		_lastFoundStart=_pageListStart;
		while (true) do
			_lastFoundStart = string.find(_searchResultHTML,"a href",_lastFoundStart+1,true);
			if (_lastFoundStart==nil or _lastFoundStart>_pageListEnd) then
				break;
			end
			_foundTotalResultPages=_foundTotalResultPages+1;
		end

		if (_foundTotalResultPages==0) then
			popupMessage("Error in get search num pages");
		end
		return _foundTotalResultPages;
	end
end

-- Given the HTML for one page of search results, return the names as the first table and the URLs as the second table
function getSingleSearchResults(_searchResultHTML)
	-- Step 1 - Get search result names and urls
	_foundUrls={};
	_foundNames={};
	i = 0;
	_lastFoundStart = string.find(_searchResultHTML,"search_result",0,true);
	while (true) do
		--<a href="/challenge/episodeList?titleNo=whateverId" class="challenge_item N=a:scl.list,i:whateverId,g:en_en">
		_lastFoundStart = string.find(_searchResultHTML,"/episodeList",_lastFoundStart+1,true);
		if (_lastFoundStart==nil) then
			break;
		end
		if (string.sub(_searchResultHTML,_lastFoundStart-16,_lastFoundStart-13)=="href") then -- Check if this string starts with /challenge
			_lastFoundStart = _lastFoundStart-10;
		end
		i = i+1;
		_foundUrlEnd = string.find(_searchResultHTML,"\"",_lastFoundStart,true);
		_foundUrls[i] = string.sub(_searchResultHTML,_lastFoundStart,_foundUrlEnd-1);
		--<p class="subj">whatever name here</p>
		_lastFoundStart = string.find(_searchResultHTML,"subj",_lastFoundStart,true);
		_lastFoundStart = _lastFoundStart+6;
		_foundNameEnd = string.find(_searchResultHTML,"<",_lastFoundStart,true);
		_foundNames[i] = string.sub(_searchResultHTML,_lastFoundStart,_foundNameEnd-1);
	end
	if (i==0) then
		_foundUrls[1]="";
		_foundNames[1]="No results";
	end
	return _foundNames, _foundUrls
end

function updateSearchLists()
	assignListData(currentQueueCLists,currentQueueCListsLength,1,searchResultNames)
	setUserInput(2,1);
end

-- Search with terms
function endSearchInput()
	goodShowStatus("Getting search results...");
	-----------------------
	lastSearchTerms = string.gsub(userInput01," ","+");
	_searchResultHTML = downloadString(string.format(SEARCHFORMATURL,lastSearchTerms,1))
	-----------------------
	searchResultNames, searchResultUrls = getSingleSearchResults(_searchResultHTML)
	unhtml(searchResultNames);
	-----------------------
	setUserInput(2,1);
	updateSearchLists()
end



function genPageListTable(_numPages)
	if (_numPages==0) then
		return {"???"}
	end
	local _returnTable = {};
	for i=1,_numPages do
		table.insert(_returnTable,tostring(i))
	end
	return _returnTable;
end

function getComicPageEpisodes(_searchResultHTML)
	--detail_body challenge
	local _pageListStart = string.find(_searchResultHTML,"detail_body",0,true);
	if (_pageListStart==nil) then
		popupMessage("Error in get comic page episodes function");
		return;
	end
	-- Apparently this shows up even for comics with only one page of episodes
	local _pageListEnd = string.find(_searchResultHTML,"paginate",_pageListStart,true);

	searchResultNames=nil;
	searchResultUrls=nil;
	searchResultNames = {};
	searchResultUrls = {};

	i=0;
	local _lastUrlStart = _pageListStart;
	while (true) do
		_lastUrlStart = string.find(_searchResultHTML,"href",_lastUrlStart+1,true);
		if (_lastUrlStart==nil or _lastUrlStart>_pageListEnd) then
			break;
		end
		i=i+1;
		_lastUrlStart = _lastUrlStart+6;
		searchResultUrls[i] = string.sub(_searchResultHTML,_lastUrlStart,string.find(_searchResultHTML,"\"",_lastUrlStart,true)-1);
		_lastUrlStart = string.find(_searchResultHTML,"subj",_lastUrlStart,true);
		_lastUrlStart = _lastUrlStart+12;
		searchResultNames[i] = string.sub(_searchResultHTML,_lastUrlStart,string.find(_searchResultHTML,"</span>",_lastUrlStart,true)-1);

		_lastUrlStart = string.find(_searchResultHTML,"</li>",_lastUrlStart,true);
	end
end

function endEpisodePageSelect()
	goodShowStatus("Getting comic episodes on page " .. userInput01);
	_searchResultHTML = downloadString(string.format(selectedComicUrl,userInput01));
	getComicPageEpisodes(_searchResultHTML);
	_searchResultHTML=nil;
	
	unhtml(searchResultNames);

	assignListData(currentQueueCLists,currentQueueCListsLength,1,searchResultNames) -- Zero based index
	setUserInput(2,1); -- Not zero based, lol
end

function unhtml(_passedTable)
	local k=0;
	local _cachedSize = #_passedTable;
	for k=1,_cachedSize do
		_passedTable[k] = fixHtmlStupidity(_passedTable[k]);
	end
end

function MyLegGuy_InputInit()
	if (currentInputScreen==1) then
		-- Fills the lists with first page of results
		goodShowStatus("Getting initial comic data...");

		_searchResultHTML = downloadString(string.format(URLPREFIX .. selectedComicUrl,1));

		-- First, libcurl follows the redirects that we get when we go to selectedComicUrl. If we want to access other pages, we need the real url. Set selectedComicUrl to the real url
		local _lastUrlStart = string.find(_searchResultHTML,"canonical",0,true);
		if (_lastUrlStart==nil) then
			popupMessage("Error in get true url")
		end
		_lastUrlStart = string.find(_searchResultHTML,"href",_lastUrlStart,true);
		_lastUrlStart=_lastUrlStart+6; -- Seek past href
		local _lastUrlEnd = string.find(_searchResultHTML,"\"",_lastUrlStart,true);
		selectedComicUrl = string.sub(_searchResultHTML,_lastUrlStart,_lastUrlEnd-1);
		-- Remove the &page=1 it ends with
		_lastUrlStart = string.find(selectedComicUrl,"&page=1",0,true);
		if (_lastUrlStart==nil) then
			popupMessage("Error in strip page off. " .. _lastUrlStart);
		end
		selectedComicUrl = (string.sub(selectedComicUrl,0,_lastUrlStart-1) .. APPENDPAGEFORMAT);


		getComicPageEpisodes(_searchResultHTML);
		numSearchResultPages = getTotalPages(_searchResultHTML);
		_searchResultHTML=nil;
	
		assignListData(currentQueueCLists,currentQueueCListsLength,0,genPageListTable(numSearchResultPages))
		assignListData(currentQueueCLists,currentQueueCListsLength,1,searchResultNames)
		setUserInput(1,1);
		setUserInput(2,1);
	end
end

function MyLegGuy_Download()
	print("Download.")

	--createDirectory(nhentaiMangaRootWithEndSlash);
	--setUserAgent("Vita-Nathan-Lua-Manga-Downloader/" .. string.format("%02d",getDownloaderVersion()));
	--doGallery(currentParsedSearchResults[userInput03])
end

function MyLegGuy_Prompt()
	-- Cannot download unless referer is set to http://www.webtoons.com
	setReferer("https://www.webtoons.com");

	--enableDownloadDebugInfo();

	-- One
	ResetUserChoices();
	userInput01="";
	userInput02=0;
	userInputQueue("Search term","Choose this. Enter the name of what you're looking for.",INPUTTYPESTRING);
	userInputQueue("Search results","Select the comic series.",INPUTTYPELIST);
	EndInput01 = endSearchInput;
	if (waitForUserInputs(false)==false or userInput01=="") then
		return false;
	end

	--------------------------------------------------------------------------------------------------
	-- These are from the last menu
	EndInput01=nil
	EndList02=nil;
	-- Save the data we need, dispose the rest
	selectedComicUrl = (searchResultUrls[userInput02] .. APPENDPAGEFORMAT);
	selectedComicName = searchResultNames[userInput02];
	searchResultUrls=nil;
	searchResultNames=nil;
	--
	currentInputScreen=1; -- So MyLegGuy_InputInit works
	EndList01 = endEpisodePageSelect;

	ResetUserChoices();
	userInputQueue("Episode page","There may be multiple pages of episodes. Choose one here. The lower the number, the more recently it was posted.",INPUTTYPELIST);
	userInputQueue("Episode","Choose episode of " .. selectedComicName .. " to download.",INPUTTYPELIST);
	if (waitForUserInputs(false)==false) then
		return false;
	end
	

	return false;
end