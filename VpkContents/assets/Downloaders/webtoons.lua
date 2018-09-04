-- one page
--https://www.webtoons.com/search?keyword=Merryweatherey&searchType=CHALLENGE
-- multiple pages
--https://www.webtoons.com/search?keyword=slice%20of%20life&searchType=CHALLENGE

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

-- The list of search result pages
function InitList01()
	local _returnTable = {};
	table.insert(_returnTable,"Search for manga.");
	table.insert(_returnTable,"Download by ID.");
	return _returnTable;
end

function MyLegGuy_Download()
	print("Download.")
	createDirectory(nhentaiMangaRootWithEndSlash);

	setUserAgent("Vita-Nathan-Lua-Manga-Downloader/" .. string.format("%02d",getDownloaderVersion()));
	doGallery(currentParsedSearchResults[userInput03])
end

function MyLegGuy_Prompt()
	-- Cannot download unless referer is set to http://www.webtoons.com
	setReferer("http://www.webtoons.com");

	-- One
	ResetUserChoices();
	userInput01="";
	userinput02=0;
	userInputQueue("Search term","Choose this. Enter the name of what you're looking for.",INPUTTYPESTRING)
	userInputQueue("Search result page","There may be multiple pages of results. Choose one here.",INPUTTYPELIST)
	userInputQueue("Search results","Select the comic series.",INPUTTYPELIST)
	userInputQueue("Episode page","There may be multiple pages of episodes. Choose one here.",INPUTTYPELIST)
	userInputQueue("Episode","Choose episode.",INPUTTYPELIST)
	if (waitForUserInputs(false)==false or userInput01=="") then
		return false;
	end

	-- Todo - Select chapters

	return false;
end