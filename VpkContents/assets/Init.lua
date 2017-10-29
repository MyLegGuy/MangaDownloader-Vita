print(_VERSION)
INPUTTYPESTRING = 1;
INPUTTYPENUMBER = 2;
INPUTTYPELIST = 3;
INPUTTYPELISTMULTI = 4;

PLAT_UNDEFINED = 0;
PLAT_WINDOWS = 1;
PLAT_VITA = 2;
PLAT_3DS = 3;

FILETYPE_JPG = 1;
FILETYPE_PNG = 2;

userInput01=nil;
userInput02=nil;
userInput03=nil;
userInput04=nil;
userInput05=nil;
userInput06=nil;
userInput07=nil;
userInput08=nil;
userInput09=nil;
userInput10=nil;

function setDoneDownloading()
	if (isAsIGo==true) then
		setMangaDoneDownloadingStatus(true);
	end
end
function sendJustDownloadedNew()
	if (isAsIGo==true) then
		incrementTotalDownloadedFiles(1);
		requireNewDirectorySearch();
	end
end

function goodJustDownloaded()
	if (isAsIGo==true) then
		incrementTotalDownloadedFiles(1);
		requireNewDirectorySearch();
	end
end

function makeFolderFriendly(str)
	-- Remove characters you can't have in folder names
	str = string.gsub(str,"~"," ");
	str = string.gsub(str,"#"," ");
	str = string.gsub(str,"%%"," ");
	str = string.gsub(str,"&"," ");
	str = string.gsub(str,"%*"," ");
	str = string.gsub(str,"{"," ");
	str = string.gsub(str,"}"," ");
	str = string.gsub(str,"\\"," ");
	str = string.gsub(str,":"," ");
	str = string.gsub(str,"<"," ");
	str = string.gsub(str,">"," ");
	str = string.gsub(str,"%?"," ");
	str = string.gsub(str,"/"," ");
	str = string.gsub(str,"%+"," ");
	str = string.gsub(str,"|"," ");
	str = string.gsub(str,"\""," ");
	return str;
end

function goodShowStatus(str)
	if (isAsIGo==false) then
		showStatus(str)
	end
end

function fixHtmlStupidity(str)
  str = string.gsub( str, '&lt;', '<' )
  str = string.gsub( str, '&gt;', '>' )
  str = string.gsub( str, '&quot;', '"' )
  str = string.gsub( str, '&apos;', "'" )
  str = string.gsub( str, '&#(%d+);', function(n) return string.char(n) end )
  str = string.gsub( str, '&#x(%d+);', function(n) return string.char(tonumber(n,16)) end )
  str = string.gsub( str, '&amp;', '&' ) -- Be sure to do this after all others
  return str
end

function ResetUserChoices()
	userInput01=nil;
	userInput02=nil;
	userInput03=nil;
	userInput04=nil;
	userInput05=nil;
	userInput06=nil;
	userInput07=nil;
	userInput08=nil;
	userInput09=nil;
	userInput10=nil;
end

-- Will return with slash by default
function getMangaFolder(endSlash)
	--print("getMangaFolder was called.")
	local _loadedMangaFolderRoot = rawGetMangaFolder();
	if (endSlash~=nil and endSlash==false) then
		return string.sub(_loadedMangaFolderRoot,1,#_loadedMangaFolderRoot-1)
	end
	return _loadedMangaFolderRoot;
end

function fixPath(path)
	path = string.gsub(path,"\\\\","\\");
	path = string.gsub(path,"//","/");
	return path;
end