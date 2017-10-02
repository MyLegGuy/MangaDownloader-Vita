SCRIPTVERSION=1
SAVEVARIABLE=2

function onOptionsLoad()
end

function InitList01(isFirstTime)
	if (isFirstTime==true) then
		good = {};
		for i=1,20 do
			table.insert(good,"(custom name " .. i .. " )");
		end
		return good;
	end
	return nil;
end

function InitList02(isFirstTime)
	if (isFirstTime==true) then
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

function MyLegGuy_Download()
	print(SCRIPTVERSION .. " is script version")
	userInputQueue("Mode","Choose \"Manga\" or \"Doujin.\"",INPUTTYPELIST)
	userInputQueue("Mode2","mode2 text here",INPUTTYPELIST)
	userInputQueue("Number","(int) Self explanatory",INPUTTYPENUMBER)
	userInputQueue("name","put your name here",INPUTTYPESTRING)
	waitForUserInputs();
	print("user choice is " .. userInput01)
end