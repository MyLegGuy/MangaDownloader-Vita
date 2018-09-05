-- one page
--https://www.webtoons.com/search?keyword=Merryweatherey&searchType=CHALLENGE
-- multiple pages
--https://www.webtoons.com/search?keyword=slice%20of%20life&searchType=CHALLENGE

SEARCHFORMATURL = "https://www.webtoons.com/search?keyword=%s&searchType=CHALLENGE&page=%d"

numSearchResultPages=0;
searchResultUrls={};
searchResultNames={};

lastSearchTerms="";

function downloadChapter(chapterUrl)
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

-- Given the HTML for a page of search results, return the total number of search result pages. Can return 1 if there's only one page
function getTotalSearchResultPages(_searchResultHTML)
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
	_lastFoundStart = 0;
	while (true) do
		--<a href="/challenge/episodeList?titleNo=whateverId" class="challenge_item N=a:scl.list,i:whateverId,g:en_en">
		_lastFoundStart = string.find(_searchResultHTML,"/challenge/episodeList",_lastFoundStart+1,true);
		if (_lastFoundStart==nil) then
			break;
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
	assignListData(currentQueueCLists,currentQueueCListsLength,1,genPageListTable(numSearchResultPages))
	assignListData(currentQueueCLists,currentQueueCListsLength,2,searchResultNames)
	setUserInput(3,1);
end

-- Search with terms
function EndInput01()
	goodShowStatus("Getting search results...");
	-----------------------
	lastSearchTerms = string.gsub(userInput01," ","+");
	_searchResultHTML = downloadString(string.format(SEARCHFORMATURL,lastSearchTerms,1))
	-----------------------
	searchResultNames, searchResultUrls = getSingleSearchResults(_searchResultHTML)
	-----------------------
	numSearchResultPages = getTotalSearchResultPages(_searchResultHTML);
	-----------------------
	setUserInput(2,1);
	updateSearchLists()
end

function EndList02()
	goodShowStatus("Getting search result page " .. userInput02);
	_searchResultHTML = downloadString(string.format(SEARCHFORMATURL,lastSearchTerms,userInput02))
	searchResultNames, searchResultUrls = getSingleSearchResults(_searchResultHTML)
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

function MyLegGuy_Download()
	print("Download.")

	--createDirectory(nhentaiMangaRootWithEndSlash);
	--setUserAgent("Vita-Nathan-Lua-Manga-Downloader/" .. string.format("%02d",getDownloaderVersion()));
	--doGallery(currentParsedSearchResults[userInput03])
end

function MyLegGuy_Prompt()
	-- Cannot download unless referer is set to http://www.webtoons.com
	setReferer("http://www.webtoons.com");

	-- One
	ResetUserChoices();
	userInput01="";
	userInput02=0;
	userInput03=0;
	userInputQueue("Search term","Choose this. Enter the name of what you're looking for.",INPUTTYPESTRING);
	userInputQueue("Search result page","There may be multiple pages of results. Choose one here.",INPUTTYPELIST);
	userInputQueue("Search results","Select the comic series.",INPUTTYPELIST);
	
	if (waitForUserInputs(false)==false or userInput01=="") then
		return false;
	end

	-- Todo - Select chapters
	userInputQueue("Episode page","There may be multiple pages of episodes. Choose one here.",INPUTTYPELIST);
	userInputQueue("Episode","Choose episode.",INPUTTYPELIST);

	

	return false;
end