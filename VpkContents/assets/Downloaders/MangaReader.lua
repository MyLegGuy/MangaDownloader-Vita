-- Specific series downloading written a long time ago. 2015 maybe? Even before that?
-- Series parsing written on 10/2/17 between 4:30 and 5:37 PM.
SCRIPTVERSION=1;
SAVEVARIABLE=0;

extraStatusPrefix="";

seriesListUrl = {};
seriesListFriendly = {};

chapterListUrl = {};
chapterListFriendly = {};

function onOptionsLoad()
	for i=1,#seriesListFriendly do
		if (seriesListFriendly[i]==userLoad01) then
			setUserInput(1,i);
			break;
		end
	end
	getChapterList(seriesListUrl[userInput01])
	setUserInput(2,0);
end

function getSeriesList()
	showStatus("Getting series list...")
	local _listHtml = downloadString("http://www.mangareader.net/alphabetical");
	-- At the top of the page, there's a bunch of letters for sorting by letter. We want to start searching past that.
	local _listStart = string.find(_listHtml,"#Z",1,true);
	local _lastFound = _listStart;
	while (true) do
		--<li><a href="/kanchigai-hime-to-usotsuki-shimobe"> Kanchigai Hime to Usotsuki Shimobe</a><span class="mangacompleted">[Completed]</span></li>
		local _liFound = string.find(_listHtml,"<li>",_lastFound+1,true);
		local _firstQuotationMark = string.find(_listHtml,"\"",_liFound+1,true);
		_lastFound = string.find(_listHtml,"\"",_firstQuotationMark+1,true);
		-- Is string, not position
		local _lastFoundUrl = string.sub(_listHtml,_firstQuotationMark+1,_lastFound-1);
		-- This url is after the end of the list.
		if (_lastFoundUrl=="http://www.animefreak.tv") then
			break;
		end
		-- Find the friendly name.
		_firstQuotationMark = string.find(_listHtml,">",_lastFound,true);
		_liFound = string.find(_listHtml,"<",_firstQuotationMark+1,true);

		local _lastFoundFriendly = string.sub(_listHtml,_firstQuotationMark+1,_liFound-1);
		-- Add to table
		table.insert(seriesListUrl,_lastFoundUrl);
		table.insert(seriesListFriendly,_lastFoundFriendly);

		-- Prepare for next loop
		_lastFound = string.find(_listHtml,"</li>",_lastFound+1,true);
	end
end

-- Returns page file name random wierd number and subdomain
function downloadGetUrlData(_mangaName,_mangaChapter,_mangaPageNumber)
	local _subdomain="";
	local _fileNumber="";

	local _startImageUrlIndex;
	local _endImageUrlIndex;
	local _currentPageHtml = downloadString("http://www.mangareader.net/" .. _mangaName .. "/" .. _mangaChapter .. "/" .. _mangaPageNumber);
	if (downloadFailed) then
		return false, false
	end
	-- First search, this is a dummy one we don't need if it's not the last page we're dealing with.
	a,b = string.find(_currentPageHtml, (".mangareader.net/" .. _mangaName .. "/" .. _mangaChapter .. "/" .. _mangaName .. "-"),1,true)
	-- If it couldn't even find that, we've failed.
	if a==nil and b==nil then
		return false,false
	end

	-- This is usually the one we want
	_startImageUrlIndex,_endImageUrlIndex = string.find(_currentPageHtml, (".mangareader.net/" .. _mangaName .. "/" .. _mangaChapter .. "/" .. _mangaName .. "-"),b,true);

	-- But if this is the last page in the chapter, that one won't be found.
	if _startImageUrlIndex==nil and _endImageUrlIndex==nil then
		-- In that case, we need the first search result.
		_startImageUrlIndex = a;
		_endImageUrlIndex = b;
		-- No longer need these.
		a=nil;
		b=nil;
	end

	-- Okay, we found the start and end index of the url we want. Let's find the number and subdomain now!
	--===================================
	-- THis is the index at which the image numer starts at.
	local numberStartLocation;
	local searchingForNumbersNumero;


	numberStartLocation=(_startImageUrlIndex+string.len(".mangareader.net/" .. _mangaName .. "/" .. _mangaChapter .. "/" .. _mangaName .. "-"))
	searchingForNumbersNumero=numberStartLocation;

	-- This will find the image number. Keep getting the next character, determine if it's a number, if so, add it on to the string and keep going.
	while true do
	if tonumber(string.sub(_currentPageHtml, searchingForNumbersNumero, searchingForNumbersNumero))~=nil then
		_fileNumber = (_fileNumber .. string.sub(_currentPageHtml, searchingForNumbersNumero, searchingForNumbersNumero))
		searchingForNumbersNumero=searchingForNumbersNumero+1
	else
	-- We've reached the end; something that's not a number.
		break
	end
	end

	-- Checks if there's a slash 3 places back. If not, 3 digit subdomain. Otherwise, 2 digit sumdomain.
	-- Do I even need 3 digit subdomain? I'm not sure.
	if (string.sub(_currentPageHtml,_startImageUrlIndex-3,_startImageUrlIndex-3)~="/") then
		_subdomain=string.sub(_currentPageHtml,_startImageUrlIndex-3,_startImageUrlIndex-1)
	else
		_subdomain=string.sub(_currentPageHtml,_startImageUrlIndex-2,_startImageUrlIndex-1)
	end

	return _fileNumber,_subdomain;
end


function downloadFindNumberOfPages()
	showStatus(extraStatusPrefix .. "Getting number of pages...")
	_mangaPageSample = downloadString("http://www.mangareader.net/" .. currentDownloadName .. "/" .. tostring(currentDownloadChapterNumber) .. "/1")
	if (downloadFailed) then
		return false;
	end
	local _checkPage = 2;

	-- 10/1/17
	-- For some reason, if you try and get a page that doesn't exist, you just see an empty page.
	-- Check for first option thing, if it doesn't exist, this is probably an empty page
	a,b = string.find(_mangaPageSample, ('<option value="/' .. currentDownloadName .. '/' .. tostring(currentDownloadChapterNumber) .. '/' .. tostring(_checkPage) .. '"'),1,true)
	if a==nil then
		popupMessage(currentDownloadName .. " doesn't exist.");
		return false;
	end

	while true do
		a,b = string.find(_mangaPageSample, ('<option value="/' .. currentDownloadName .. '/' .. tostring(currentDownloadChapterNumber) .. '/' .. tostring(_checkPage) .. '"'),1,true)
		if a==nil then
			break;
		end
		_checkPage=_checkPage+1;
	end
	currentDownloadTotalPages=_checkPage-1;
end

-- Downloads
function downloadPage(_mangaName,_mangaChapter,_mangaPageNumber,_saveLocation)
	local _subdomain;
	local _fileNumber;
	_fileNumber, _subdomain = downloadGetUrlData(_mangaName,_mangaChapter,_mangaPageNumber);
	showStatus(extraStatusPrefix .. "Downloading " .. _mangaPageNumber .. "/" .. currentDownloadTotalPages);
	downloadFile(("http://" .. _subdomain .. ".mangareader.net/" .. _mangaName .. "/" .. _mangaChapter .. "/" .. _mangaName .. "-" .. _fileNumber .. ".jpg"),(_saveLocation .. string.format("%03d",_mangaPageNumber) .. ".jpg"));
end

function makeFolders()
	createDirectory(getMangaFolder() .. currentDownloadName)
	createDirectory(getMangaFolder() .. currentDownloadName .. "/chapter-" .. string.format("%03d",currentDownloadChapterNumber))
end

-- 10/1/17 10:38 PM - 10:55 PM
function downloadCover()
	if (shouldDownloadCovers()==true) then
		if (fileExists(getMangaFolder() .. currentDownloadName .. "/cover.jpg")==false) then
			local _coverHtml = downloadString("http://www.mangareader.net/" .. currentDownloadName);
			local lastFound=-1;
			--<div id="mangaimg"><img src="http://s1.mangareader.net/cover/the-gamer/the-gamer-l0.jpg" alt="The Gamer Manga"></div>
			lastFound = string.find(_coverHtml,"mangaimg",lastFound+1,true);
			local firstQuotationMark = string.find(_coverHtml,"\"",lastFound+11,true);
			lastFound = string.find(_coverHtml,"\"",firstQuotationMark+1,true);
			downloadFile((string.sub(_coverHtml,firstQuotationMark+1,lastFound-1)),(getMangaFolder() .. currentDownloadName .. "/cover.jpg"))
		end
	end
end

function downloadDoTheThing()
	if (downloadFindNumberOfPages()==false) then
		return;
	end
	downloadCover();
	makeFolders()
	for i=1,currentDownloadTotalPages do
		downloadPage(currentDownloadName,currentDownloadChapterNumber,i,currentDownloadSaveLocation)
	end
end

function initializeDownloadMenu()
	currentDownloadChapterNumber=1;
	currentDownloadName="inuyasha";
	currentDownloadSubdomain="Working...";
	currentDownloadSaveLocation=(getMangaFolder() .. currentDownloadName .. "/chapter-" .. string.format("%03d",currentDownloadChapterNumber) .. "/");
end
function InitList01(isFirstTime)
	if (isFirstTime>=1) then
		return seriesListFriendly;
	end
	return nil;
end



function InitList02(isFirstTime)
	if (isFirstTime>=1) then
		return chapterListFriendly
	end
	return nil;
end

function EndList01()
	getChapterList(seriesListUrl[userInput01])
	if (#chapterListFriendly>0) then
		assignListData(currentQueueCLists,currentQueueCListsLength,1,chapterListFriendly)
	end
end

function getChapterList(seriesname)
	showStatus("Getting chapter list...")
	chapterListUrl = {};
	chapterListFriendly = {};
	local _listHtml = downloadString("http://www.mangareader.net" .. seriesname);
	local _lastFound = string.find(_listHtml,"chapterlist",1,true);
	local i=1;
	while (true) do
		--[[
			<tr>
			<td>
			<div class="chico_manga"></div>
			<a href="/naruto/1">Naruto 1</a> : Uzumaki Naruto</td>
			<td>07/04/2009</td>
			</tr>
		]]
		local _linkStart = string.find(_listHtml,"<a href=",_lastFound+1,true);
		local _firstQuotationMark = string.find(_listHtml,"\"",_linkStart+1,true);
		_lastFound = string.find(_listHtml,"\"",_firstQuotationMark+1,true);
		local _lastFoundUrl = string.sub(_listHtml,_firstQuotationMark+1,_lastFound-1);
		if (_lastFoundUrl=="http://www.animefreak.tv") then
			break;
		end
		_lastFoundUrl = string.match(_lastFoundUrl,".*/(.*)")
		table.insert(chapterListUrl,_lastFoundUrl);
		--_firstQuotationMark = string.find(_listHtml,">",_lastFound+1,true);
		--_lastFound = string.find(_listHtml,"<",_firstQuotationMark+1,true);
		--_lastFoundUrl = string.sub(_listHtml,_firstQuotationMark+1,_lastFound-1);
		table.insert(chapterListFriendly,tostring(i));
		i=i+1
	end
end

function MyLegGuy_Download()
	ResetUserChoices();
	getSeriesList();
	userInputQueue("Series","Choose the manga series.",INPUTTYPELIST);
	userInputQueue("Chapter","(int) Self explanatory",INPUTTYPELISTMULTI)
	if (waitForUserInputs(1)==false) then
		return;
	end
	--local _mangaNameToDownload = "";
	--local _chapterNumberToDownload=0;
	--userInputQueue("Name","Input the name of the manga from URL.\nFor example, \"naruto\" from http://www.mangareader.net/naruto/19\nFor example, \"naruto\" from http://www.mangareader.net/naruto",INPUTTYPESTRING)
	--userInputQueue("Chapter","(int) Self explanatory",INPUTTYPENUMBER)
	--if (waitForUserInputs(1)==false) then
	--	return;
	--end

	for i=1,#userInput02 do
		extraStatusPrefix = ("Chapter " .. i .. "/" .. tostring(#userInput02) .. "\n" .. "\"" .. chapterListUrl[userInput02[i]] .. "\"" .. "\n");
		_mangaNameToDownload = seriesListUrl[userInput01];
		-- Remove starting slash
		_mangaNameToDownload = string.sub(_mangaNameToDownload,2);
		_chapterNumberToDownload = tonumber(userInput02[i]);
		_chapterNumberToDownload = math.floor(_chapterNumberToDownload);
		initializeDownloadMenu()
		currentDownloadName = _mangaNameToDownload
		currentDownloadChapterNumber = _chapterNumberToDownload
		currentDownloadSaveLocation=(getMangaFolder() .. currentDownloadName .. "/chapter-" .. string.format("%03d",currentDownloadChapterNumber) .. "/");
		downloadDoTheThing()
	end

	
end