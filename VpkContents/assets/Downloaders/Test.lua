SCRIPTVERSION=1;
SAVEVARIABLE=0;

function MyLegGuy_Download()
	-- userInput01
end

function MyLegGuy_Prompt()
	happy = loadImageFromUrl("https://www.google.com/images/branding/googlelogo/1x/googlelogo_color_272x92dp.png",FILETYPE_PNG)
	photoViewer(happy)
	freeTexture(happy)

	userInputQueue("name","put your name here",INPUTTYPESTRING)
	waitForUserInputs();
end