-- Started on 10/19/17
-- Very basic gallery downloading by ID made by 8:50 PM 10/19/17
-- Searching made on 10/20/17 by 11:59 PM
-- Fixed no search result bug on 8/18/18 1:29 AM

-- TODO - Make it so user can tell what's in English

SCRIPTVERSION=1;
SAVEVARIABLE=0;

nhentaiMangaRootWithEndSlash = getMangaFolder(true) .. "nhentai/"
-- MODE_SEARCH variables
currentParsedSearchResults = {};

MODE_SEARCH = 1;
MODE_ID = 2;
userChosenMode=0;

NORESULTSTRING = "No results!";

function storage_onListMoreInfo(_passedListId, _passedListEntry)
	if ((_passedListId)==3) then
		_parsedGallery = currentParsedSearchResults[_passedListEntry];
		goodShowStatus("Getting cover...");
		--downloadFile("https://t.nhentai.net/galleries/" .. _parsedGallery.media_id .. "/cover" .. "." .. convertFileExtention(_parsedGallery.coverFormat),_fixedFolderName .. "/cover." .. convertFileExtention(_parsedGallery.coverFormat))
		
		if (convertFileExtention(_parsedGallery.coverFormat)=="jpg") then
			_tempLoadedCover = loadImageFromUrl("https://t.nhentai.net/galleries/" .. _parsedGallery.media_id .. "/cover" .. "." .. convertFileExtention(_parsedGallery.coverFormat),FILETYPE_JPG)
		elseif (convertFileExtention(_parsedGallery.coverFormat)=="png") then
			_tempLoadedCover = loadImageFromUrl("https://t.nhentai.net/galleries/" .. _parsedGallery.media_id .. "/cover" .. "." .. convertFileExtention(_parsedGallery.coverFormat),FILETYPE_PNG)
		else
			popupMessage("Invalid format. " .. _parsedGallery.coverFormat);
			return;
		end
		photoViewer(_tempLoadedCover)
		-- Alredy freed by photo viewer
		--freeTexture(_tempLoadedCover)
	end
end

function convertFileExtention(_abreviation)
	if (_abreviation=="j") then
		return "jpg";
	elseif (_abreviation=="p") then
		return "png";
	else
		print("Unknown file extention!");
		print(_abreviation)
		print("Just printed.")
		return "jpg";
	end
end

function getGalleryById(_passedFriendlyId)
	print("https://nhentai.net/api/gallery/" .. _passedFriendlyId)
	showStatus("Getting gallery...")
	local _downloadedGalleryJSON = downloadString("https://nhentai.net/api/gallery/" .. _passedFriendlyId);
	showStatus("Checking for connection timeout...")
	if (string.find(_downloadedGalleryJSON,"Connection timed out",0,true)~=nil) then
		popupMessage("Connection timed out.");
		return false;
	end
	return _downloadedGalleryJSON;
end
-- Parses gallery nearest to supplied index
-- Returns table
function parseGalleryString(_downloadedGalleryJSON, _startIndex)
	local _parsedGallery = {};
	_parsedGallery.pagesFormat = {};
	_parsedGallery.coverFormat="";
	_parsedGallery.num_pages=0;
	_parsedGallery.media_id="0";

	local _firstFind=0;
	local _secondFind=0;

	-- First, look for the gallery media id
	_firstFind = string.find(_downloadedGalleryJSON,"media_id",_startIndex,true);
	-- 11 chars after the start of media_id is the start of the number
	_secondFind = string.find(_downloadedGalleryJSON,"\"",_firstFind+11,true);
	-- Found media_id
	_parsedGallery.media_id = string.sub(_downloadedGalleryJSON,_firstFind+11,_secondFind-1);

	-- Find the pretty name
	-- "pretty":"Festa, Festa, Festa!!! DereMas Soushuuhen"
	--_firstFind = string.find(_downloadedGalleryJSON,"\"pretty\":",_secondFind,true);
	--_secondFind = string.find(_downloadedGalleryJSON,"\"",_firstFind+11,true);
	--_parsedGallery.prettyName = fixUtf8InString(string.sub(_downloadedGalleryJSON,_firstFind+10,_secondFind-1));

	_parsedGallery.prettyName = fixUtf8InString(parseEscapable(_downloadedGalleryJSON,string.find(_downloadedGalleryJSON,"\"pretty\":",_secondFind,true)+10));

	-- Find number of pages
	_firstFind = string.find(_downloadedGalleryJSON,"num_pages",_secondFind+10,true);

	local _numPageEnd = string.find(_downloadedGalleryJSON,",",_firstFind+11,true);
	local _possibleAltEnd = string.find(_downloadedGalleryJSON,"}",_firstFind+11,true);
	-- Whichever ends the array first
	if (_possibleAltEnd~=nil) then
		if (_numPageEnd==nil or _possibleAltEnd<_numPageEnd) then
			_numPageEnd=_possibleAltEnd;
		end
	else
		if (_numPageEnd==nil) then
			popupMessage("Error parse with _numPageEnd, will now crash.");
		end
	end
	_parsedGallery.num_pages = tonumber(string.sub(_downloadedGalleryJSON,_firstFind+11,_numPageEnd-1));

	-- This is the start of the image table. After we find this index, we'll start searching for pages starting from here
	_firstFind = string.find(_downloadedGalleryJSON,"\"images\"",_secondFind,true);
	
	-- Find the file format of the cover
	_firstFind = string.find(_downloadedGalleryJSON,"\"t\"",_firstFind,true);
	_parsedGallery.coverFormat = string.sub(_downloadedGalleryJSON,_firstFind+5,_firstFind+5);

	-- Find all file extentions
	for i=1,_parsedGallery.num_pages do
		_firstFind = string.find(_downloadedGalleryJSON,"\"t\"",_firstFind+1,true);
		_parsedGallery.pagesFormat[i] = string.sub(_downloadedGalleryJSON,_firstFind+5,_firstFind+5);
	end
	return _parsedGallery;
end

function parseSearchResults(_searchResultJSON)
	--local _searchResultJSON = downloadString("https://nhentai.net/api/galleries/search?query=naruto&page=1");
	local _lastFoundUploadDateIndex=0;

	local _totalParsedResults = {};
	local i=1;
	while true do
		local _tempFound = string.find(_searchResultJSON,"upload_date",_lastFoundUploadDateIndex+1,true);
		if (_tempFound==nil) then
			break;
		end
		showStatus("Parsing result " .. i)
		table.insert(_totalParsedResults,parseGalleryString(_searchResultJSON,_lastFoundUploadDateIndex));
		i=i+1;
		_lastFoundUploadDateIndex = _tempFound;
	end
	if (i==1) then
		_totalParsedResults.num_pages = 0;
		return _totalParsedResults;
	end

	-- If we're here, that means that no more mangas were found.
	-- _lastFoundUploadDateIndex contains the start of the last found manga.
	-- This will find the number of pages in the last manga
	_lastFoundUploadDateIndex = string.find(_searchResultJSON,"num_pages",_lastFoundUploadDateIndex,true);
	-- Because we're sure the last result was the last manga, we can be sure that the next num_pages is the number of pages in the search.
	_lastFoundUploadDateIndex = string.find(_searchResultJSON,"num_pages",_lastFoundUploadDateIndex+1,true);
	-- If this is null, then there were no search results. The num_pages that was found before this one was the one reporting that there are 0 search results.
	if (_lastFoundUploadDateIndex==nil) then
		_totalParsedResults.num_pages = 0;
		return _totalParsedResults;
	end
	--],"num_pages":1,"per_page":25}
	_totalParsedResults.num_pages = tonumber(string.sub(_searchResultJSON,_lastFoundUploadDateIndex+11,string.find(_searchResultJSON,",",_lastFoundUploadDateIndex+11,true)-1));
	return _totalParsedResults;
end

-- Converts all \uxxxx stuff it finds
function fixUtf8InString(str)
	local _lastFoundUtf8Offset = 0;
	while (true) do
		_lastFoundUtf8Offset = string.find(str,"\\u",0,true);
		if (_lastFoundUtf8Offset==nil) then
			break;
		end
		str = string.sub(str,1,_lastFoundUtf8Offset-1) .. utf8.char(tonumber(string.sub(str,_lastFoundUtf8Offset+2,_lastFoundUtf8Offset+5),16)) .. string.sub(str,_lastFoundUtf8Offset+6);
	end
	return str;
end

_currentSearchParsedPage=-1;
_list02UserInput01Cache = "";
function InitList02(isFirstTime)
	-- Only need to refresh this if search term is changed.
	if (userInput01~=_list02UserInput01Cache) then
		setUserInput(2,1);
		_list02UserInput01Cache = userInput01;
		local _returnTable = {};
		if (currentParsedSearchResults.num_pages==0) then
			table.insert(_returnTable,NORESULTSTRING);
		else
			for i=1,currentParsedSearchResults.num_pages do
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
	currentParsedSearchResults = parseSearchResults(_searchResultJSON);
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
				table.insert(_returnTable,(currentParsedSearchResults[i].prettyName));
			end
		end
		return _returnTable;
	end
	return nil;
end

function getSearch(_passedSearchTerms, _passedPageNumber)
	showStatus("Getting search JSON...")
	_passedSearchTerms = string.gsub(_passedSearchTerms," ","+");
	return downloadString("https://nhentai.net/api/galleries/search?query=" .. _passedSearchTerms .. "&page=" .. _passedPageNumber)
end

function getGalleryFullFolder(_parsedGallery)
	return nhentaiMangaRootWithEndSlash .. makeFolderFriendly(_parsedGallery.prettyName);
end

function doGallery(_parsedGallery)
	-- Manga specific directory
	local _fixedFolderName = getGalleryFullFolder(_parsedGallery);
	createDirectory(_fixedFolderName);

	-- Download the cover beforehand
	goodShowStatus("Downloading cover...");
	downloadFile("https://t.nhentai.net/galleries/" .. _parsedGallery.media_id .. "/cover" .. "." .. convertFileExtention(_parsedGallery.coverFormat),_fixedFolderName .. "/cover." .. convertFileExtention(_parsedGallery.coverFormat))
	
	-- Download the pages
	for i=1,_parsedGallery.num_pages do
		local _fileExtention = convertFileExtention(_parsedGallery.pagesFormat[i]);
		goodShowStatus(_parsedGallery.prettyName .. "\n" .. i .. "/" .. _parsedGallery.num_pages);
		downloadFile("https://i.nhentai.net/galleries/" .. _parsedGallery.media_id .. "/" .. i .. "." .. _fileExtention,_fixedFolderName .. "/" .. string.format("%03d",i) .. "." .. _fileExtention);
		sendJustDownloadedNew();
	end
	setDoneDownloading();
end

function MyLegGuy_Download()
	print("Download.")
	createDirectory(nhentaiMangaRootWithEndSlash);

	setUserAgent("Vita-Nathan-Lua-Manga-Downloader/" .. string.format("%02d",getDownloaderVersion()));
	doGallery(currentParsedSearchResults[userInput03])
end

-- Only called for the first prompt because the second prompt has a string for the first input.
function InitList01()
	local _returnTable = {};
	table.insert(_returnTable,"Search for manga.");
	table.insert(_returnTable,"Download by ID.");
	return _returnTable;
end

function MyLegGuy_Prompt()
	__tempHoldEndInput1 = EndInput01;
	EndInput01=nil;

	ResetUserChoices();
	userInputQueue("Mode","Will you use an ID, or search?",INPUTTYPELIST);
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
		userInputQueue("ID","The ID. For example, 212113 from https://nhentai.net/g/212113/",INPUTTYPESTRING)
		if (waitForUserInputs(false)==false) then
			return false;
		end
		currentParsedSearchResults = {};
		table.insert(currentParsedSearchResults,parseGalleryString(getGalleryById(userInput01),0));
		userInput03=1;
	end
	if (isAsIGo==true) then
		_asIgoFolder="";
		_asIgoFolder = getGalleryFullFolder(currentParsedSearchResults[userInput03]);
		_asIgoFolder = _asIgoFolder .. "/"
	end
	return true;
end