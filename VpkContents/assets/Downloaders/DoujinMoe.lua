-- Written on 9/2/17
SCRIPTVERSION=1;

--https://doujins.com/zr63zbpp
--downloadFile("https://doujins.com/zr63zbpp","./sampledoujin.html");
--downloadFile("https://dev.doujins.com/n-jh9uvmap.jpg?st=ahvf4Cp_1vd2vc6TiuU2tw&e=1504383722","samplepage.jpg")

-- What is this?! Some kind of p 0rn downloader?!
-- yes.
-- string.find is inclusive?
function MyLegGuy_Download()
	ResetUserChoices();

	local _urlEndName="";
	userInputQueue("ID","Choose a doujin, go to the overview page where you can see all the pages, and input what you find at the end of the url. For example, \"pgpmysn2\" from https://doujins.com/pgpmysn2",INPUTTYPESTRING)
	if (waitForUserInputs()==false) then
		return;
	end
	_urlEndName = userInput01;

	createDirectory(getMangaFolder() .. "DoujinMoe")
	
	showStatus("Getting pages HTML...");
	local completedUrl = "https://doujins.com/" .. _urlEndName;
	print("Downloading from " .. completedUrl)
	local webpageDownloaded = downloadString(completedUrl);
	local _firstGalleryLocation = string.find(webpageDownloaded,"gallery",1,true); -- A div with the ID "gallery" marks the start of the gallery.

	-- zr63zbpp results
	-- last commented images
	-- something for contorl bar or something
	-- Link that shows the doujin's name that leads to zr63zbpp

	showStatus("Parsing pages HTML...");
	local _nameTagStart=1;
	for i=1,3 do
		_nameTagStart = string.find(webpageDownloaded,_urlEndName,_nameTagStart+1,true);
	end
	local _doujinName = string.sub(webpageDownloaded,_nameTagStart+string.len(_urlEndName)+2,string.find(webpageDownloaded,"<",_nameTagStart,true)-1);
	showStatus("Fixing folder name...");
	-- Remove characters you can't have in folder names
	_doujinName = string.gsub(_doujinName,"~"," ");
	_doujinName = string.gsub(_doujinName,"#"," ");
	_doujinName = string.gsub(_doujinName,"%%"," ");
	_doujinName = string.gsub(_doujinName,"&"," ");
	_doujinName = string.gsub(_doujinName,"%*"," ");
	_doujinName = string.gsub(_doujinName,"{"," ");
	_doujinName = string.gsub(_doujinName,"}"," ");
	_doujinName = string.gsub(_doujinName,"\\"," ");
	_doujinName = string.gsub(_doujinName,":"," ");
	_doujinName = string.gsub(_doujinName,"<"," ");
	_doujinName = string.gsub(_doujinName,">"," ");
	_doujinName = string.gsub(_doujinName,"%?"," ");
	_doujinName = string.gsub(_doujinName,"/"," ");
	_doujinName = string.gsub(_doujinName,"%+"," ");
	_doujinName = string.gsub(_doujinName,"|"," ");
	_doujinName = string.gsub(_doujinName,"\""," ");
	
	createDirectory(getMangaFolder() .. "DoujinMoe/" .. _doujinName)
	local _currentSearchingIndex = _firstGalleryLocation;
	showStatus("Starting first page...");
	for i=1,999 do
		if (_currentSearchingIndex==nil) then
			break;
		end
		local _openDjmTag = string.find(webpageDownloaded,"djm",_currentSearchingIndex+1,true);
		if (_openDjmTag==nil) then
			break;
		end
		showStatus("Downloading page " .. i);
		local _endFirstDjmString = string.find(webpageDownloaded,"\"",_openDjmTag+10,true);
		local _foundUrl = string.sub(webpageDownloaded,_openDjmTag+10,_endFirstDjmString-1);
		print("Page " .. i);
		downloadFile(_foundUrl,getMangaFolder() .. "DoujinMoe/" .. _doujinName .. "/" .. string.format("%03d",i) .. ".jpg")
		_currentSearchingIndex = string.find(webpageDownloaded,"djm",_openDjmTag+1,true);
	end
end