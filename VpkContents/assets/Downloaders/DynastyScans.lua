SCRIPTVERSION=1
SAVEVARIABLE=0
-- DoTheStuff was programmed in 12/7/16 in a C# program
-- Other code was written starting on 9/5/17
-- As I go mode integration started 10/14/17 12:28 AM
urlPrefix = "https://dynasty-scans.com";
completeDownloadFolderName="";

seriesListFriendly = {};
seriesListUrl = {};

chapterListUrl = {};
chapterListFriendly = {};

function onOptionsLoad()
	-- Reset chapter select list
	setUserInput(2,0);
	-- Load series
	for i=1,#seriesListFriendly do
		if (seriesListFriendly[i]==userLoad01) then
			setUserInput(1,i);
			break;
		end
	end
	--if (#chapterListFriendly==0) then
	GetChapterList(seriesListUrl[userInput01])
	--end
end


function onListMoreInfo__stuff(_passedListId, _passedListEntry)
	if ((_passedListId)==1) then
		if (isDoujin==true) then
			popupMessage("None avalible.")
		end
		showStatus("Getting cover...")
		_tempLoadedCover = loadImageFromUrl(getCoverUrl(seriesListUrl[_passedListEntry]),FILETYPE_JPG)
		photoViewer(_tempLoadedCover)
		freeTexture(_tempLoadedCover)
	end
end

--https:--dynasty-scans.com/xxxx
--Stores the completed xxxx part
--Should be something like "chapters/new_game_ch32"
-- Folder name should not end in slash
function DoTheStuff(completeMangaNameUrlSuffix,completeDownloadFolderName,_statusStringPrefix)
	if (isAsIGo==false) then
		showStatus("Getting pages...");
	end
	local suffix = completeMangaNameUrlSuffix;
	print("Downloading " .. suffix);

	-- Suffix. User changes this.
	--string suffix = "unemployed_and_high_school_girl";
	-- We find links to da good stuff in here.
	local htmlCode;
	htmlCode = downloadString(urlPrefix .. suffix);
	--print(htmlCode);
	local startA = htmlCode:find("var pages = ");
	local numPages = 0;
	local pastPosition = startA;
	local tempA;
	while (true) do
		tempA = htmlCode:find("}", pastPosition + 1);
		if ((tempA) ~= nil) then
			numPages = numPages+1;
			pastPosition = tempA;
		else
			break;
		end
	end
	-- Holds urls for all the pages
	urls = {};

	-- startA is very start.
	-- tempA is just a temp variable.
	local lastQuotationMark=startA;
	local firstQuotationMark;

	-- This sets the lastquotation mark to the last quotation mark for the first image.
	tempA = startA;
	tempA = htmlCode:find('"', tempA + 1, true);
	tempA = htmlCode:find('"', tempA + 1, true);
	tempA = htmlCode:find('"', tempA + 1, true);
	tempA = htmlCode:find('"', tempA + 1, true);
	lastQuotationMark = tempA;

	for i=2,numPages do
		-- Gets url and puts it in da place
		tempA = lastQuotationMark;
		for i=1,6 do
			tempA = htmlCode:find('"', tempA + 1);
		end
		firstQuotationMark=htmlCode:find('"', tempA + 1,true);
		lastQuotationMark=htmlCode:find('"', firstQuotationMark + 1,true);
		urls[i] = htmlCode:sub(firstQuotationMark + 1, (firstQuotationMark + 1)+(lastQuotationMark - firstQuotationMark - 1)-1);
	end

	tempA = startA;
	tempA = htmlCode:find('"', tempA + 1);
	tempA = htmlCode:find('"', tempA + 1);
	-- First quotation mark (left) for file url
	local tempC = htmlCode:find('"', tempA + 1);
	-- Second quotation mark (right) for file url
	local tempB = htmlCode:find('"', tempC + 1);
	urls[1] = htmlCode:sub(tempC + 1, (tempC + 1)+(tempB - tempC - 1)-1);
	--print("Base url: " + baseUrl);
	local realFileUrlPrefix = "https://dynasty-scans.com";

	local fileFormat = urls[1]:sub(string.len(urls[1]) - 3, (string.len(urls[1]) - 4)+4);
	for i=1,numPages do
		fileFormat = urls[i]:sub(string.len(urls[i]) - 3, (string.len(urls[i]) - 4)+4)
		if (isAsIGo==false) then
			showStatus(_statusStringPrefix .. "Downloading " .. (i) .. "/" .. numPages);
		end
		downloadFile(realFileUrlPrefix .. urls[i], completeDownloadFolderName .. "/" .. string.format("%03d",i) .. fileFormat);
		incrementTotalDownloadedFiles(1);
		requireNewDirectorySearch();
	end
	if (isAsIGo==true) then
		setMangaDoneDownloadingStatus(true);
	end
end
function GetSeriesList(_retrieveSeriesUrl)
	-- <dd><a href="/series/1_x">1 x %</a></dd>
	showStatus("Getting series list...");
	GetDDInfo(_retrieveSeriesUrl,seriesListUrl,seriesListFriendly);
end
function GetDDInfo(url, tableUrl, tableFriendly)
	local _lastDownloadedHtml = downloadString(url);
	local lastFound=-1;
	while (true) do
		-- Find url name
		lastFound = string.find(_lastDownloadedHtml,"<dd>",lastFound+1,true);
		if (lastFound==nil) then
			break;
		end
		local foundQuotationMark = string.find(_lastDownloadedHtml,"\"",lastFound+1,true);
		lastFound = string.find(_lastDownloadedHtml,"\"",foundQuotationMark+1,true);
		table.insert(tableUrl,fixHtmlStupidity(string.sub(_lastDownloadedHtml,foundQuotationMark+1,lastFound-1)));
		-- Find friendly name
		local foundLeftBracket = string.find(_lastDownloadedHtml,">",lastFound+1,true);
		lastFound = string.find(_lastDownloadedHtml,"<",foundLeftBracket+1,true);
		table.insert(tableFriendly,fixHtmlStupidity(string.sub(_lastDownloadedHtml,foundLeftBracket+1,lastFound-1)));
	end
end
function GetChapterList(chosenSeries)
	--<a href="/chapters/yuyushiki_ch08" class="name">Chapter 8</a>
	chapterListUrl={};
	chapterListFriendly={};
	showStatus("Getting chapter list...");
	GetDDInfo("https://dynasty-scans.com" .. chosenSeries,chapterListUrl,chapterListFriendly);
end
function input1_InitList01(isFirstTime)
	_returnTable = {};
	table.insert(_returnTable,"Manga");
	table.insert(_returnTable,"Doujin");
	return _returnTable;
end
function input2_InitList01(isFirstTime)
	-- If the user is just selecting this normally, reset their chapter choice.
	-- isFirstTime value of 2 says that this called because of loading an options file. If an options file is loaded, it's definetly a valid chapter.
	if (isFirstTime~=2) then
		-- Reset chapter selection
		setUserInput(2,1);
	end
	if (isFirstTime>=1) then
		return seriesListFriendly;
	end
	return nil;
end
function input2_EndList01()
	GetChapterList(seriesListUrl[userInput01])
	if (#chapterListFriendly>0) then
		assignListData(currentQueueCLists,currentQueueCListsLength,1,chapterListFriendly)
	end
end

function input2_InitList02(isFirstTime)
	if (userInput01~=nil) then
		if (isFirstTime>=1) then
			return chapterListFriendly;
		end
	end
	return nil;
end

function getCoverUrl(coverSeriesPartUrl)
	local _coverHtml = downloadString("https://dynasty-scans.com" .. coverSeriesPartUrl);
	local lastFound=-1;
	--<img alt="Yuyushiki_04" class="thumbnail" src="/system/tag_contents_covers/000/002/527/medium/Yuyushiki_04.jpg?1371254739">
	lastFound = string.find(_coverHtml,"thumbnail",lastFound+1,true);
	local firstQuotationMark = string.find(_coverHtml,"\"",lastFound+11,true);
	lastFound = string.find(_coverHtml,"\"",firstQuotationMark+1,true);
	return ("https://dynasty-scans.com" .. string.sub(_coverHtml,firstQuotationMark+1,lastFound-1));
end

function DownloadCover(downloadLocation, coverSeriesPartUrl)
	if (shouldDownloadCovers()==true) then
		if (fileExists(downloadLocation .. "/cover.jpg")==false) then
			downloadFile(getCoverUrl(coverSeriesPartUrl),(downloadLocation .. "/cover.jpg"))
		end
	end
end
function boolToNumber(_bol)
	if (_bol==true) then
		return 1;
	else
		return 0;
	end
end

function MyLegGuy_Prompt()
	ResetUserChoices();
	-- Let user choose between doujin or manga
	InitList01 = input1_InitList01;
	userInputQueue("Mode","Choose \"Manga\" or \"Doujin.\"",INPUTTYPELIST);
	if (waitForUserInputs(false)==false) then
		return;
	end
	isDoujin=true;
	if (userInput01==1) then
		isDoujin=false;
	elseif (userInput01==2) then
		isDoujin=true;
	else
		print("Bad userInput01 value! is:")
		print(userInput01);
		print("plz fix!");
		isDoujin=false;
	end
	ResetUserChoices();
	-- Let user choose series and chapter or series and doujn
	InitList01 = input2_InitList01;
	InitList02 = input2_InitList02;
	EndList01 = input2_EndList01;
	onListMoreInfo = onListMoreInfo__stuff;
	if (isDoujin==false) then
		GetSeriesList("https://dynasty-scans.com/series");
		userInputQueue("Series","The manga name. Selecting this option will reset your selected chapter.",INPUTTYPELIST);
		if (isAsIGo==true) then
			userInputQueue("Chapter","The specific chapter of the manga. Specials will also be here.",INPUTTYPELIST);
		else
			userInputQueue("Chapter","The specific chapter of the manga. Specials will also be here.",INPUTTYPELISTMULTI);
		end
		SAVEVARIABLE = boolToNumber(isDoujin);
		if (waitForUserInputs(1)==false) then
			return;
		end
	elseif (isDoujin==true) then
		GetSeriesList("https://dynasty-scans.com/doujins");
		userInputQueue("Series","The manga this doujin is based on. Selecting this option will reset your selected doujin.",INPUTTYPELIST);
		if (isAsIGo==true) then
			userInputQueue("Doujin","The specific doujin of the manga.",INPUTTYPELIST);
		else
			userInputQueue("Doujin","The specific doujin of the manga.",INPUTTYPELISTMULTI);
		end
		SAVEVARIABLE = boolToNumber(isDoujin);
		if (waitForUserInputs(2)==false) then
			return;
		end
	end
	if (isAsIGo==true) then
		local _cachedUserInput = userInput02;
		userInput02 = {};
		userInput02[1] = _cachedUserInput;
		local tempSeriesFolder = fixPath(getMangaFolder(true) .. string.match(seriesListUrl[userInput01],".*/(.*)"));
		_asIgoFolder = fixPath(tempSeriesFolder .. "/" .. string.match(chapterListUrl[userInput02[1]],".*/(.*)") .. "/");
	end
end

function MyLegGuy_Download()
	-- Make folder for the manga's series
	-- Fancy string matching will get the manga name from /series/yuyushiki
	local tempSeriesFolder = fixPath(getMangaFolder(true) .. string.match(seriesListUrl[userInput01],".*/(.*)"));
	--chapterListUrl = nil;
	--chapterListFriendly = nil;
	createDirectory(tempSeriesFolder);
	if (isDoujin==false) then
		DownloadCover(tempSeriesFolder, seriesListUrl[userInput01]);
	end
	for i=1,#userInput02 do
		local tempMangaNameUrlSuffix = chapterListUrl[userInput02[i]];
		-- Chapter specific folder
		local tempCompleteDownloadFolderName = fixPath(tempSeriesFolder .. "/" .. string.match(chapterListUrl[userInput02[i]],".*/(.*)"));
		createDirectory(tempCompleteDownloadFolderName);
		DoTheStuff(tempMangaNameUrlSuffix,tempCompleteDownloadFolderName,("Chapter " .. i .. "/" .. tostring(#userInput02) .. "\n" .. "\"" .. chapterListFriendly[userInput02[i]] .. "\"" .. "\n"))
	end
end