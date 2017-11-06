// TODO - Include everything that my 3ds one had.
// TODO - More site support. I made a list.
	// Please fix nh before, though.
#define VERSION 2

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
// main.h
	char popupMessage(const char* _tempMsg, char _waitForAButton, char _isQuestion);
// Max number of downloader scripts and pages in manga reader.
#define MAXFILES 200

#define TEMPDEBUGMODE 0
#define FORCEDOWNLOADMODE 1

#define FILETYPE_JPG 1
#define FILETYPE_PNG 2

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
void WriteToDebugFile(const char* stuff);
#include "photo.h"
#include "Downloader.h"

/////////////////////////////////////////////////////////
char popupMessage(const char* _tempMsg, char _waitForAButton, char _isQuestion){
	controlsEnd();
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
			if (textWidth(fontSize,&(message[lastNewlinePosition+1]))>screenWidth-20){
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
	if (textWidth(fontSize,&(message[lastNewlinePosition+1]))>screenWidth-20){
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
	controlsEnd();
	_numberOfLines-=currentlyVisibleLines;
	do{
		FpsCapStart();
		controlsStart();
		int _lastStrlen=0;
		if (wasJustPressed(SCE_CTRL_CROSS)){
			if (_numberOfLines<=0){
				if (_isQuestion==1){
					controlsEnd();
					return 1;
				}
				break;
			}
			offsetStrlen += strlen(&message[offsetStrlen])+1;
			_numberOfLines--;
		}else if (wasJustPressed(SCE_CTRL_CIRCLE)){
			if (_isQuestion==1){
				controlsEnd();
				return 0;
			}
		}
		controlsEnd();
		startDrawing();
		// We need this variable so we know the offset in the message for the text that is for the next line
		_lastStrlen=0;
		for (i=0;i<currentlyVisibleLines;i++){
			goodDrawTextColored(5,textHeight(fontSize)*i,&message[_lastStrlen+offsetStrlen],fontSize,COLORSTATUS);
			// This offset will have the first letter for the next line
			_lastStrlen = strlen(&message[_lastStrlen+offsetStrlen])+1+_lastStrlen;
			if (_lastStrlen>=totalMessageLength){
				break;
			}
		}
		endDrawing();
		FpsCapWait();
	}while(_waitForAButton==1);
	controlsEnd();
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
	fixPath(CONSTANTFONTFILE,tempPathFixBuffer,TYPE_EMBEDDED);
	loadFont(tempPathFixBuffer);
	currentTextHeight = textHeight(fontSize);
	cursorWidth = textWidth(fontSize,">");
	// Make data folder
	fixPath("",tempPathFixBuffer,TYPE_DATA);
	createDirectory(tempPathFixBuffer);

	#if PLATFORM == PLAT_VITA
		// Magic fix for joysticks
		sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
	#endif
}

signed char mainMenuSelection(){
	char* _tempList[3];
	//_tempList[0]="Read";
	_tempList[0]="Do nothing";
	_tempList[1]="Download";
	_tempList[2]="Exit";
	return showList(_tempList, 3, 0, NULL)-1;
}

int main(int argc, char *argv[]){
	init();
	initDownloadBroad();

	//while (1){
		#if FORCEDOWNLOADMODE==1
			signed char _lastMainMenuSelectioin=1;
		#else
			signed char _lastMainMenuSelectioin = mainMenuSelection();
		#endif
		if (_lastMainMenuSelectioin==0){ // Main read

		}else if (_lastMainMenuSelectioin==1){ // Download
			#if TEMPDEBUGMODE == 0
				char* _noobList[2];
				_noobList[0] = "Download and wait";
				_noobList[1] = "Download as I go";
				signed char _isAsIGo = showList(_noobList, 2, 0, NULL)-1;
				if (_isAsIGo==-1){
					return 0;
				}
			#else
				printf("Is temp debug mode.\n");
				char _isAsIGo=0;
			#endif
			char* _chosenScript = chooseScript();
			if (_chosenScript==NULL){
				return 0;
			}
			doScript(_chosenScript,_isAsIGo);
		}else{ // The exit option and if the user cancels
			return 0;
		}
	//}

	
	return 0;
}
