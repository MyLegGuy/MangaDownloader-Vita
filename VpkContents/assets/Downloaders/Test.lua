SCRIPTVERSION=1
SAVEVARIABLE=2

function onOptionsLoad()
end

function InitList01(isFirstTime)
	if (isFirstTime==1) then
		good = {};
		for i=1,20 do
			table.insert(good,"(custom name " .. i .. " )");
		end
		return good;
	end
	return nil;
end

function InitList02(isFirstTime)
	if (isFirstTime==1) then
		good = {};
		table.insert(good,"a");
		table.insert(good,"b");
		table.insert(good,"c");
		return good;
	else
		good = {};
		table.insert(good,"d");
		table.insert(good,"e");
		table.insert(good,"f");
		return good;
	end
end

function InitList03(isFirstTime)
	_returnList = {};
	table.insert(_returnList,"shortname")
	table.insert(_returnList,"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz")
	table.insert(_returnList,"abcdefghijklmnopqrstuvwxyz")
	table.insert(_returnList,"hi")
	return _returnList;
end

function MyLegGuy_Download()
	print(SCRIPTVERSION .. " is script version")
	userInputQueue("Mode","Choose \"Manga\" or \"Doujin.\"",INPUTTYPELISTMULTI)
	userInputQueue("Mode2","mode2 text here",INPUTTYPELIST)
	userInputQueue("Mode3","big list test",INPUTTYPELIST)
	userInputQueue("Number","(int) Self explanatory",INPUTTYPENUMBER)
	userInputQueue("name","put your name here",INPUTTYPESTRING)
	waitForUserInputs();
	print("user choice is " .. userInput01)
end