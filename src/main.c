#define VERSION 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
// main.h
	char popupMessage(const char* _tempMsg, char _waitForAButton, char _isQuestion);
// Max number of downloader scripts and pages in manga reader.
#define MAXFILES 200

#include "GeneralGoodConfig.h"
#include "GeneralGoodExtended.h"
#include "GeneralGood.h"
#include "GeneralGoodGraphics.h"
#include "GeneralGoodText.h"
#include "GeneralGoodImages.h"
#include "FpsCapper.h"
#include "KeyboardCode.h"
#include "OpenBSDstrcharstr.h"
#include "LinkedList.h"
int currentTextHeight=0;
int screenHeight;
int screenWidth;
int cursorWidth;
#include "photo.h"
#include "Downloader.h"

/////////////////////////////////////////////////////////
#define TEXTBOXY 0
char popupMessage(const char* _tempMsg, char _waitForAButton, char _isQuestion){
	ControlsEnd();
	// The string needs to be copied. We're going to modify it, at we can't if we just type the string into the function and let the compiler do everything else
	char message[strlen(_tempMsg)+1+strlen("\n(X for YES, O for NO)")];
	strcpy(message,_tempMsg);
	if (_isQuestion==1){
		strcat(message,"\n(X for YES, O for NO)");
	}
	int totalMessageLength = strlen(message);
	int i, j;
	signed short _numberOfLines=1;
	// This will loop through the entire message, looking for where I need to add new lines. When it finds a spot that
	// needs a new line, that spot in the message will become 0. So, when looking for the place to 
	int lastNewlinePosition=-1; // If this doesn't start at -1, the first character will be cut off
	for (i = 0; i < totalMessageLength; i++){
		if (message[i]==32){ // Only check when we meet a space. 32 is a space in ASCII
			message[i]='\0';
			if (TextWidth(fontSize,&(message[lastNewlinePosition+1]))>screenWidth-20){
				char _didWork=0;
				for (j=i-1;j>lastNewlinePosition+1;j--){
					//printf("J:%d\n",j);
					if (message[j]==32){
						message[j]='\0';
						_didWork=1;
						message[i]=32;
						lastNewlinePosition=j;
						_numberOfLines++;
						break;
					}
				}
				if (_didWork==0){
					message[i]='\0';
					lastNewlinePosition=i+1;
					_numberOfLines++;
				}
			}else{
				message[i]=32;
			}
		}
	}
	// This code will make a new line if there needs to be one because of the last word
	if (TextWidth(fontSize,&(message[lastNewlinePosition+1]))>screenWidth-20){
		for (j=totalMessageLength-1;j>lastNewlinePosition+1;j--){
			if (message[j]==32){
				message[j]='\0';
				_numberOfLines++;
				break;
			}
		}
	}
	char currentlyVisibleLines=screenHeight/currentTextHeight;
	// This variable is the location of the start of the first VISIBLE line
	// This will change if the text box has multiple screens because the text is too long
	int offsetStrlen=0;
	//  textboxNewCharSpeed
	ControlsEnd();
	_numberOfLines-=currentlyVisibleLines;
	do{
		FpsCapStart();
		ControlsStart();
		int _lastStrlen=0;
		if (WasJustPressed(SCE_CTRL_CROSS)){
			if (_numberOfLines<=0){
				if (_isQuestion==1){
					ControlsEnd();
					return 1;
				}
				break;
			}
			offsetStrlen += strlen(&message[offsetStrlen])+1;
			_numberOfLines--;
		}else if (WasJustPressed(SCE_CTRL_CIRCLE)){
			if (_isQuestion==1){
				ControlsEnd();
				return 0;
			}
		}
		ControlsEnd();
		StartDrawing();
		// We need this variable so we know the offset in the message for the text that is for the next line
		_lastStrlen=0;
		for (i=0;i<currentlyVisibleLines;i++){
			GoodDrawTextColored(5,TEXTBOXY+TextHeight(fontSize)*i,&message[_lastStrlen+offsetStrlen],fontSize,COLORSTATUS);
			// This offset will have the first letter for the next line
			_lastStrlen = strlen(&message[_lastStrlen+offsetStrlen])+1+_lastStrlen;
			if (_lastStrlen>=totalMessageLength){
				break;
			}
		}
		EndDrawing();
		FpsCapWait();
	}while(_waitForAButton==1);
	ControlsEnd();
	return 0;
}

void WriteToDebugFile(const char* stuff){
	#if PLATFORM == PLAT_VITA
		FILE *fp;
		fp = fopen("ux0:data/LUAMANGAS/a.txt", "a");
		fprintf(fp,"%s\n",stuff);
		fclose(fp);
	#else
		printf("Write: %s\n",stuff);
	#endif
}
// Does not clear the debug file at ux0:data/LUAMANGAS/a.txt  , I promise.
void ClearDebugFile(){
	#if PLATFORM == PLAT_VITA
		FILE *fp;
		fp = fopen("ux0:data/LUAMANGAS/a.txt", "w");
		fclose(fp);
	#endif
}
void WriteIntToDebugFile(int a){
	#if PLATFORM == PLAT_VITA
		FILE *fp;
		fp = fopen("ux0:data/LUAMANGAS/a.txt", "a");
		fprintf(fp,"%d\n", a);
		fclose(fp);
	#else
		printf("Write: %d\n",a);
	#endif
}

/////////////////////////////////////////////////////////
void init(){
	ClearDebugFile();
	initGraphics(640,480, &screenWidth, &screenHeight);
	// Text
	FixPath(CONSTANTFONTFILE,tempPathFixBuffer,TYPE_EMBEDDED);
	LoadFont(tempPathFixBuffer);
	currentTextHeight = TextHeight(fontSize);
	cursorWidth = TextWidth(fontSize,">");
	// Make data folder
	FixPath("",tempPathFixBuffer,TYPE_DATA);
	createDirectory(tempPathFixBuffer);
}
int main(int argc, char *argv[]){
	init();
	// Magic fix for joysticks
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
	initDownloadBroad();
	doScript(chooseScript(),1);
	// Free filenames, excluding the one we need. TODO
	//int i;
	//for (i=0;_mangaDirectoryFilenames[i]!=NULL;i++){
	//	free(_mangaDirectoryFilenames[i]);
	//}

	//initDownloadBroad();
	//doScript(chooseScript());
	return 0;
}
/*
TODO - The bug happens when I'm trying to move to the next page.
There is a 0 byte file and the game crashes. It is currently unknown
why it scans, finds, and tries to go to a zero byte file.

TODO - Add elegant LUA method called like setIsDone();
to tell main program when manga is finished downloading.
*/