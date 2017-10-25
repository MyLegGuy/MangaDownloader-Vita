SCRIPTVERSION=1;
SAVEVARIABLE=0;

function onListMoreInfo(listId, listEntry)
	print("List: " .. listId)
	print("Entry: " .. listEntry)
end

function InitList01(isFirstTime)
	_returnTable = {};
	for i=1,10 do
		table.insert(_returnTable,tostring(i));
	end
	return _returnTable
end

function MyLegGuy_Download()
	-- userInput01
	print("Download here.")
end

function MyLegGuy_Prompt()
	--happy = loadImageFromUrl("https://www.google.com/images/branding/googlelogo/1x/googlelogo_color_272x92dp.png",FILETYPE_PNG)
	--photoViewer(happy)
	--freeTexture(happy)

	userInputQueue("name","put your name here",INPUTTYPELIST)
	waitForUserInputs();
end