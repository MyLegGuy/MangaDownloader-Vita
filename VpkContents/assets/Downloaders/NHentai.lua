-- Started on 10/19/17
-- Very basic gallery downloading by ID made by 8:50 PM 10/19/17
-- Searching made on 10/20/17 by 11:59 PM
-- Fixed no search result bug on 8/18/18 1:29 AM
-- RIP API. Redone 3/4/19

SCRIPTVERSION=2;
SAVEVARIABLE=0;

nhentaiMangaRootWithEndSlash = getMangaFolder(true) .. "nhentai/"
-- MODE_SEARCH variables
currentParsedSearchResults = {};
numResultPages=1;
cachedFromAsIGo=nil;

MODE_SEARCH = 1;
MODE_ID = 2;
userChosenMode=0;

NORESULTSTRING = "No results!";


function getDelimThing(_passedHtml, _passedMarker, _startPos, _deliminator)
	local _quotationStart = string.find(_passedHtml,_passedMarker,_startPos,true);
	if (_quotationStart==nil) then
		return nil, _startPos;
	end
	_quotationStart = _quotationStart + string.len(_passedMarker);
	return string.sub(_passedHtml,_quotationStart,string.find(_passedHtml,_deliminator,_quotationStart,true)-1), _quotationStart;
end
-- for example,
-- bla="stuff you want"
-- you'd pass bla=\"
-- and get
-- stuff you want
function getQuotationMarkThing(_passedHtml, _passedMarker, _startPos)
	return getDelimThing(_passedHtml,_passedMarker,_startPos,"\"")
end
function getFileExtension(_passedPath)
	return string.sub(_passedPath,findCharReverse(_passedPath,".")+1);
end


--[[
i: https://t.nhentai.net/galleries/xxxxxxx/1t.jpg
o: https://i.nhentai.net/galleries/xxxxxxx/1.jpg
]]
function thumbToRealUrl(_passedUrl)
	local _dotStination = findCharReverse(_passedUrl,".");
	return "https://i" .. string.sub(_passedUrl,10,_dotStination-2) .. string.sub(_passedUrl,_dotStination);
end
function getGalleryInfo(_passedFullUrl)
	local _parsedUrls = {};
	local _galleryName;

	goodShowStatus("Getting HTML...");
	local _pagehtml = downloadString(_passedFullUrl);
	goodShowStatus("Parsing...");
	local _lastThumbLoc = -1;
	while(true) do
		_lastThumbLoc = string.find(_pagehtml,"thumb-container",_lastThumbLoc+1,true);
		if (_lastThumbLoc==nil) then
			break;
		end
		table.insert(_parsedUrls,thumbToRealUrl(getQuotationMarkThing(_pagehtml,"img src=\"",_lastThumbLoc)));
	end

	local _nameStart = string.find(_pagehtml,"itemprop=\"name\"",0,true);
	local _name, _nameStart = getQuotationMarkThing(_pagehtml,"content=\"",_nameStart);
	local _coverUrl = getQuotationMarkThing(_pagehtml,"content=\"",_nameStart);
	return _parsedUrls, _coverUrl, _name
end

function storage_onListMoreInfo(_passedListId, _passedListEntry)
	if ((_passedListId)==3) then
		_parsedGallery = currentParsedSearchResults[_passedListEntry];
		if (_parsedGallery~=nil) then
			goodShowStatus("Getting cover...");
			--downloadFile("https://t.nhentai.net/galleries/" .. _parsedGallery.media_id .. "/cover" .. "." .. convertFileExtention(_parsedGallery.coverFormat),_fixedFolderName .. "/cover." .. convertFileExtention(_parsedGallery.coverFormat))
			
			local _gottenExtension = getFileExtension(currentParsedSearchResults[_passedListEntry].coverUrl)
			if (_gottenExtension=="jpg") then
				_tempLoadedCover = loadImageFromUrl(_parsedGallery.coverUrl,FILETYPE_JPG)
			elseif (_gottenExtension=="png") then
				_tempLoadedCover = loadImageFromUrl(_parsedGallery.coverUrl,FILETYPE_PNG)
			else
				popupMessage("Invalid format. " .. _gottenExtension);
				return;
			end
			photoViewer(_tempLoadedCover)
			-- Alredy freed by photo viewer
			--freeTexture(_tempLoadedCover)
		end
	end
end


function parseSearchResults(_searchResultJSON)
	showStatus("Parsing results...");

	local _retTable = {}
	local _resultLoc = string.find(_searchResultJSON,"index-container",0,true);
	if (_resultLoc~=nil) then
		while(true) do
			_resultLoc = string.find(_searchResultJSON,"div class=\"gallery\"",_resultLoc+1,true);
			if (_resultLoc==nil) then
				break;
			end
			local _currentEntry = {};
			_currentEntry.url, _resultLoc = getQuotationMarkThing(_searchResultJSON,"a href=\"",_resultLoc)
			_currentEntry.coverUrl, _resultLoc = getQuotationMarkThing(_searchResultJSON,"img src=\"",_resultLoc);
			_currentEntry.name, _resultLoc = getDelimThing(_searchResultJSON,"div class=\"caption\">",_resultLoc,"</div>");
			if (string.sub(_currentEntry.coverUrl,1,2)=="//") then
				_currentEntry.coverUrl = "https:" .. _currentEntry.coverUrl
			end
			table.insert(_retTable,_currentEntry);
		end
	
	
		local _afterButtonPosition = string.find(_searchResultJSON,"class=\"last\"",0,true);
		local _beforePageButton = findCharReverse(_searchResultJSON,"?",_afterButtonPosition);
		local _maxPageString = getQuotationMarkThing(_searchResultJSON,"page=",_beforePageButton)
		return _retTable, tonumber(_maxPageString);
	else
		return {},0
	end
end

-- Converts all \uxxxx stuff it finds
--function fixUtf8InString(str)
--	local _lastFoundUtf8Offset = 0;
--	while (true) do
--		_lastFoundUtf8Offset = string.find(str,"\\u",0,true);
--		if (_lastFoundUtf8Offset==nil) then
--			break;
--		end
--		str = string.sub(str,1,_lastFoundUtf8Offset-1) .. utf8.char(tonumber(string.sub(str,_lastFoundUtf8Offset+2,_lastFoundUtf8Offset+5),16)) .. string.sub(str,_lastFoundUtf8Offset+6);
--	end
--	return str;
--end

_currentSearchParsedPage=-1;
_list02UserInput01Cache = "";
function InitList02(isFirstTime)
	-- Only need to refresh this if search term is changed.
	if (userInput01~=_list02UserInput01Cache) then
		setUserInput(2,1);
		_list02UserInput01Cache = userInput01;
		local _returnTable = {};
		if (#currentParsedSearchResults==0) then
			table.insert(_returnTable,NORESULTSTRING);
		else
			for i=1,numResultPages do
				table.insert(_returnTable,tostring(i));
			end
		end
		return _returnTable;
	end
	return nil;
end

function EndInput01()
	updateSearchWithUserInputs();
	_currentSearchParsedPage=userInput02;
	setUserInput(2,1);
	setUserInput(3,1);
	assignListData(currentQueueCLists,currentQueueCListsLength,1,InitList02(1))
	assignListData(currentQueueCLists,currentQueueCListsLength,2,InitList03(1))
end

function EndList02()
	if (_currentSearchParsedPage~=userInput02) then
		if (_currentSearchParsedPage~=userInput02) then
			updateSearchWithUserInputs();
			_currentSearchParsedPage = userInput02
			assignListData(currentQueueCLists,currentQueueCListsLength,2,InitList03(1))
		end
	end
end

function updateSearchWithUserInputs()
	currentParsedSearchResults={};
	_currentSearchParsedPage = userInput02;
	local _searchResultJSON = getSearch(userInput01,math.floor(userInput02));
	currentParsedSearchResults, numResultPages = parseSearchResults(_searchResultJSON);
end



_userInput01Cache = "";
_cacheSearchPage=-1;
function InitList03(isFirstTime)
	-- Only need to refresh this if page or search term is changed.
	if (userInput01~=_userInput01Cache or userInput02 ~= _cacheSearchPage) then
		_userInput01Cache = userInput01;
		_cacheSearchPage = userInput02;
		local _returnTable = {};
		local _cachedNumber = #currentParsedSearchResults;
		if (_cachedNumber==0) then
			table.insert(_returnTable,NORESULTSTRING);
		else
			for i=1,_cachedNumber do
				table.insert(_returnTable,(currentParsedSearchResults[i].name));
				--table.insert(_returnTable,"[" .. currentParsedSearchResults[i].langPrefix .. "]" ..(currentParsedSearchResults[i].prettyName));
			end
		end
		return _returnTable;
	end
	return nil;
end

function getSearch(_passedSearchTerms, _passedPageNumber)
	showStatus("Getting search JSON...")
	_passedSearchTerms = string.gsub(_passedSearchTerms," ","+");
	return downloadString("https://nhentai.net/search/?q=" .. _passedSearchTerms .. "&page=" .. _passedPageNumber)
end

function getGalleryFullFolder(_passedName)
	return nhentaiMangaRootWithEndSlash .. makeFolderFriendly(_passedName);
end

function doGallery(_passedUrl)
	local _gottenPageUrls;
	local _gottenName;
	local _gottenCover;
	
	--[[
	cachedFromAsIGo.urlTable = _gottenUrlTable;
		cachedFromAsIGo.name = _gottenName;
		cachedFromAsIGo.urlTable,cachedFromAsIGo.coverUrl,cachedFromAsIGo.name
		]]
	if (cachedFromAsIGo~=nil) then
		_gottenPageUrls = cachedFromAsIGo.urlTable;
		_gottenName = cachedFromAsIGo.name;
		_gottenCover = cachedFromAsIGo.coverUrl;
	else
		_gottenPageUrls,_gottenCover,_gottenName = getGalleryInfo(_passedUrl);
	end

	-- Manga specific directory
	local _fixedFolderName = getGalleryFullFolder(_gottenName);
	createDirectory(_fixedFolderName);

	--
	goodShowStatus("Downloading cover...");
	downloadFile(_gottenCover,_fixedFolderName .. "/cover." .. getFileExtension(_gottenCover));

	-- Download the pages
	for i=1,#_gottenPageUrls do
		goodShowStatus(_gottenName .. "\n" .. i .. "/" .. #_gottenPageUrls);
		downloadFile(_gottenPageUrls[i],_fixedFolderName .. "/" .. string.format("%03d",i) .. "." .. getFileExtension(_gottenPageUrls[i]));
		sendJustDownloadedNew();
	end
	setDoneDownloading();
end

function MyLegGuy_Download()
	print("Download.")
	createDirectory(nhentaiMangaRootWithEndSlash);

	setUserAgent("Vita-Nathan-Lua-Manga-Downloader/" .. string.format("%02d",getDownloaderVersion()));
	doGallery("https://nhentai.net" .. currentParsedSearchResults[userInput03].url)
end

-- Only called for the first prompt because the second prompt has a string for the first input.
function InitList01()
	local _returnTable = {};
	table.insert(_returnTable,"Search for manga.");
	table.insert(_returnTable,"Download by ID.");
	return _returnTable;
end

function MyLegGuy_Prompt()
	local __tempHoldEndInput1 = EndInput01;
	EndInput01=nil;

	ResetUserChoices();
	userInputQueue("Mode","Will you use an ID from a URL, or search?",INPUTTYPELIST);
	if (waitForUserInputs(false)==false) then
		return false;
	end
	userChosenMode = userInput01;

	if (userChosenMode == MODE_SEARCH) then
		onListMoreInfo = storage_onListMoreInfo;
		EndInput01 = __tempHoldEndInput1;
		ResetUserChoices();
		userInput01="";
		userInput02=1;
		userInput03=1;
		userInputQueue("Search","Search term.",INPUTTYPESTRING)
		userInputQueue("Search result page","There may be multiple pages of results. Choose one here.",INPUTTYPELIST)
		userInputQueue("Search result","After searching, choose a result.",INPUTTYPELIST)
		if (waitForUserInputs(false)==false) then
			return false;
		end
		if (#currentParsedSearchResults==0 or currentParsedSearchResults[userInput03]==nil) then
			print("Invalid inputs.")
			return false;
		end
	elseif (userChosenMode == MODE_ID) then
		ResetUserChoices();
		userInputQueue("ID","The ID. For example, 999999 from https://nhentai.net/g/999999/",INPUTTYPESTRING)
		if (waitForUserInputs(false)==false) then
			return false;
		end
		if (tonumber(userInput01)==nil) then
			popupMessage("Invalid input. Should be a number.");
			return false;
		end
		local _fakeResultEntry = {};
		_fakeResultEntry.url = "/g/" .. userInput01 .. "/";
		currentParsedSearchResults = {};
		table.insert(currentParsedSearchResults,_fakeResultEntry);
		userInput03=1;
	end
	if (isAsIGo==true) then
		showStatus("Getting folder...");

		cachedFromAsIGo = {};
		cachedFromAsIGo.urlTable,cachedFromAsIGo.coverUrl,cachedFromAsIGo.name = getGalleryInfo("https://nhentai.net" .. currentParsedSearchResults[userInput03].url);

		_asIgoFolder="";
		_asIgoFolder = getGalleryFullFolder(cachedFromAsIGo.name);
		_asIgoFolder = _asIgoFolder .. "/"
	end
	return true;
end