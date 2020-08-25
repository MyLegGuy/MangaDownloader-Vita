/*
  Copyright (C) 2018 MyLegGuy

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
/*
TODO - Option to use uma0.
TODO - Include everything that my 3ds one had.
TODO - More site support. I made a list.
TODO - To make things faster, add the option to remember asIgo selection
TODO - Ability to delete all .lastSelection files
*/
#define VERSION 4
#define VERSIONSTRING "v2.5"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
// Max number of downloader scripts and pages in manga reader.
#define MAXFILES 255

#define TEMPDEBUGMODE 0
#define FORCEDOWNLOADMODE 0

#define FILETYPE_JPG 1
#define FILETYPE_PNG 2

#include "GeneralGoodConfig.h"
#include "GeneralGoodExtended.h"
#include "GeneralGood.h"
#include "GeneralGoodGraphics.h"
#include "GeneralGoodText.h"
#include "GeneralGoodImages.h"
#include "main.h"
#include "fpsCapper.h"
#include "keyboardCode.h"
#include "openBSDstrcharstr.h"
#include "linkedList.h"
int currentTextHeight=0;
int screenHeight;
int screenWidth;
int cursorWidth;
#include "photo.h"
#include "Downloader.h"
char* dataFolderRoot;

char* ANDROIDPACKAGENAME = "com.mylegguy.lua.manga";
// 9 characters
char* VITAAPPID = "LUAMANGAS";
/////////////////////////////////////////////////////////
void XOutFunction(){
	exit(0);
}
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
// Doesn't actually get extention. Just returns last few characters of string.
char* getFileExtention(char* _filename, int _extentionLength){
	if (strlen(_filename)<_extentionLength){
		return _filename;
	}
	return &(_filename[strlen(_filename)-_extentionLength]);
}
char hasImageExtension(char* _fullPath){
	char* _ext = getFileExtention(_fullPath,3);
	return (strcmp(_ext,"png")==0 || strcmp(_ext,"jpg")==0);
}
static char getImageType(unsigned char _magicStart){
	if (_magicStart==0x89){
		return 1;
	}else if (_magicStart==0xFF){
		return 2;
	}/*else if (_magicStart==0x42){
		return 3;
		}*/else{
		return 0;
	}
}
CrossTexture* loadLoadableImage(char* path){
	FILE* fp=fopen(path,"rb");
	if (fp==NULL){
		return NULL;
	}
	unsigned char _magicStart = fgetc(fp);
	fclose(fp);
	switch(getImageType(_magicStart)){
		case 1:
			return loadPNG(path);
		case 2:
			return loadJPG(path);
			/*case 3:
			  return loadBMP(path);*/
	}
	return NULL;
}
// Must be a malloc'd list
void alphabetizeList(char** _passedList,int _totalFileListLength){
	int i,j;
	for (i = 0; i < _totalFileListLength-1 ; i++){ // minus one because no need to check last file
		for (j = i; j < _totalFileListLength; j++){
			if (strcmp(_passedList[i], _passedList[j]) > 0){ // Move up next one if less than this one
				char* _tempBuffer = malloc(strlen(_passedList[i])+1);
				strcpy(_tempBuffer, _passedList[i]);
				free(_passedList[i]);
				_passedList[i] = malloc(strlen(_passedList[j])+1);
				strcpy(_passedList[i],_passedList[j]);
				free(_passedList[j]);
				_passedList[j] = _tempBuffer;
			}
		}
	}
}
signed char mainMenuSelection(){
	char* _tempList[3];
	_tempList[0]="Read";
	//_tempList[0]="Do nothing";
	_tempList[1]="Download";
	_tempList[2]="Exit";
	return showList(_tempList, 3, 0, NULL)-1;
}
// excludes .lastSelection
char** getDirectory(char* _path, int* _lengthStorage){
	NathanLinkedList* _foundFileList = calloc(1,sizeof(NathanLinkedList));
	int i;
	// Select download script
	CROSSDIR dir;
	CROSSDIRSTORAGE lastStorage;
	dir = openDirectory (_path);
	if (dirOpenWorked(dir)==0){
		popupMessage("Failed to open directory.",1,0);
		return NULL;
	}
	int _totalFileListLength=0;
	for (i=0;;i++){
		if (directoryRead(&dir,&lastStorage) == 0){
			break;
		}
		NathanLinkedList* _currentListEntry = addToLinkedList(_foundFileList);
		_currentListEntry->memory = malloc(strlen(getDirectoryResultName(&lastStorage))+1);
		strcpy(_currentListEntry->memory,getDirectoryResultName(&lastStorage));
		_totalFileListLength++;
	}
	directoryClose (dir);
	if (_totalFileListLength==0){
		return NULL;
	}
	removeFromLinkedList(&_foundFileList,searchLinkedList(_foundFileList,".lastSelection"));
	// Just in case one is removed.
	_totalFileListLength = getLinkedListLength(_foundFileList);
	char** _directoryFilenameList = (char**)linkedListToArray(_foundFileList);
	alphabetizeList(_directoryFilenameList,_totalFileListLength);
	if (_lengthStorage!=NULL){
		*_lengthStorage=_totalFileListLength;
	}
	return _directoryFilenameList;
}
// realloc, but new memory is zeroed out
void* recalloc(void* _oldBuffer, int _newSize, int _oldSize){
	void* _newBuffer = realloc(_oldBuffer,_newSize);
	if (_newSize > _oldSize){
		void* _startOfNewData = ((char*)_newBuffer)+_oldSize;
		memset(_startOfNewData,0,_newSize-_oldSize);
	}
	return _newBuffer;
}
void freeMallocdArray(void** _array, int _arrayLength){
	int i;
	for (i=0;i<_arrayLength;i++){
		free(_array[i]);
	}
	free(_array);
}
// /a/b/c/
// /a/b/a.png
// both will go to
// /a/b/
char* backADirectory(char* _filepath){
	int i;
	for (i=strlen(_filepath)-2;i>0;i--){ // Start at length minus two because we don't want to detect a slash that's the last character
		if (_filepath[i]==47 || _filepath[i]==58){
			int j;
			int _cachedStringLength = strlen(_filepath);
			for (j=i+1;j!=_cachedStringLength;j++){
				_filepath[j]=0;
			}
			break;
		}
	}
	return _filepath;
}
char stringStartsWith(char* _bigString, char* _shortString){
	return strncmp(_bigString, _shortString, strlen(_shortString))==0;
}

void createLastSelectedFile(char* _passedDirectoryWithSlash, char* _passedFilename){
	char _tempFilepathComplete[strlen(_passedDirectoryWithSlash)+strlen(".lastSelection")+1];
	strcpy(_tempFilepathComplete,_passedDirectoryWithSlash);
	strcat(_tempFilepathComplete,".lastSelection");
	FILE* fp = fopen(_tempFilepathComplete,"w");
	fprintf(fp,"%s",_passedFilename);
	fclose(fp);
}
char* loadLastSelectedFile(char* _passedDirectoryWithSlash){
	char _tempFilepathComplete[strlen(_passedDirectoryWithSlash)+strlen(".lastSelection")+1];
	strcpy(_tempFilepathComplete,_passedDirectoryWithSlash);
	strcat(_tempFilepathComplete,".lastSelection");
	if (checkFileExist(_tempFilepathComplete)==1){
		char _tempBuffer[100];
		FILE* fp = fopen(_tempFilepathComplete,"r");
		fgets(_tempBuffer,100,fp);
		fclose(fp);
		char* _returnBuffer = malloc(strlen(_tempBuffer)+1);
		strcpy(_returnBuffer,_tempBuffer);
		return _returnBuffer;
	}else{
		return NULL;
	}
}
// 0 based return value
// Returns -1 if not found
signed int searchCharArray(char** _passedArray, int _passedLength, char* _searchTerm){
	int i;
	for (i=0;i<_passedLength;i++){
		if (strcmp(_passedArray[i],_searchTerm)==0){
			return i;
		}
	}
	return -1;
}
/////////////////////////////////////////////////////////
void init(){
	ClearDebugFile();
	initGraphics(640,480, &screenWidth, &screenHeight);
	
	// Text
	fontSize=30;
	#if PLATFORM != PLAT_VITA
		fixPath("assets/LiberationSans-Regular.ttf",tempPathFixBuffer,TYPE_EMBEDDED);
		loadFont(tempPathFixBuffer);
	#else
		loadFont("sa0:data/font/pvf/jpn0.pvf");
	#endif
	currentTextHeight = textHeight(fontSize);
	cursorWidth = textWidth(fontSize,">");
	// Make data folder
	fixPath("",tempPathFixBuffer,TYPE_DATA);
	createDirectory(tempPathFixBuffer);
	dataFolderRoot = malloc(strlen(tempPathFixBuffer)+1);
	strcpy(dataFolderRoot,tempPathFixBuffer);
	#if PLATFORM == PLAT_VITA
		// Magic fix for joysticks
		sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
	#endif
}
void mainRead(char* _startingDirectory){
	char* _currentDirectoryPath = malloc(strlen(_startingDirectory)+1);
	strcpy(_currentDirectoryPath,_startingDirectory);
	while(1){
		int _currentDirectoryLength;
		signed int _foundStartingSelection=0;
		char** _currentDirectoryListing = getDirectory(_currentDirectoryPath,&_currentDirectoryLength);
		if (_currentDirectoryListing==NULL){
			_currentDirectoryPath = backADirectory(_currentDirectoryPath);
			continue;
		}
		char* _tempLoadedLastSelected = loadLastSelectedFile(_currentDirectoryPath);
		if (_tempLoadedLastSelected!=NULL){
			_foundStartingSelection=searchCharArray(_currentDirectoryListing,_currentDirectoryLength,_tempLoadedLastSelected);
			if (_foundStartingSelection==-1){
				_foundStartingSelection=0;
			}
			free(_tempLoadedLastSelected);
		}
		signed int _tempUserChoice = showList(_currentDirectoryListing,_currentDirectoryLength,_foundStartingSelection,NULL)-1;
		if (_tempUserChoice==-2){ // Normally -1, but we subtracted 1.
			_currentDirectoryPath = backADirectory(_currentDirectoryPath);
			// Go back if user exits data directory
			if (!stringStartsWith(_currentDirectoryPath,dataFolderRoot)){
				break;
			}
			continue;
		}else{
			createLastSelectedFile(_currentDirectoryPath,_currentDirectoryListing[_tempUserChoice]);
		}
		_currentDirectoryPath = recalloc(_currentDirectoryPath,strlen(_currentDirectoryPath)+1+strlen(_currentDirectoryListing[_tempUserChoice])+1,strlen(_currentDirectoryPath)+1);
		strcat(_currentDirectoryPath,_currentDirectoryListing[_tempUserChoice]);
		
		// Test if single image
		if (hasImageExtension(_currentDirectoryPath)){
			_currentDirectoryPath = backADirectory(_currentDirectoryPath);
			currentDownloadReaderDirectory = _currentDirectoryPath;
			isDoneDownloading=1;
			needUpdateFileListing=1;
			totalDownloadedFiles=-1;
			char* _tempArgument = malloc(strlen(_currentDirectoryListing[_tempUserChoice])+1);
			strcpy(_tempArgument,_currentDirectoryListing[_tempUserChoice]);
			char* _pageReadTo = photoViewer(NULL,_tempArgument); // We want the last selected file to be the last page the user read
			if (_pageReadTo!=NULL){
				createLastSelectedFile(_currentDirectoryPath,_pageReadTo);
				free(_pageReadTo);
			}
			free(_tempArgument);
		}else{
			strcat(_currentDirectoryPath,"/");
		}
		freeMallocdArray((void**)_currentDirectoryListing,_currentDirectoryLength);
	}
	free(_currentDirectoryPath);
	return;
}

int main(int argc, char *argv[]){
	init();
	initDownloadBroad();
	while (1){
		#if FORCEDOWNLOADMODE==1
			signed char _lastMainMenuSelectioin=1;
		#else
			signed char _lastMainMenuSelectioin = mainMenuSelection();
		#endif
		if (_lastMainMenuSelectioin==0){ // Main read
			fixPath("",tempPathFixBuffer,TYPE_DATA);
			char* _tempArgument = malloc(strlen(tempPathFixBuffer)+1);
			strcpy(_tempArgument,tempPathFixBuffer); // Be safe. Don't want tempPathFixBuffer to be changed while we're using it.
			mainRead(_tempArgument);
			free(_tempArgument);
		}else if (_lastMainMenuSelectioin==1){ // Download
			#if TEMPDEBUGMODE == 0
				char* _noobList[2];
				_noobList[0] = "Download and wait";
				_noobList[1] = "Download as I go";
				signed char _isAsIGo = showList(_noobList, 2, 0, NULL)-1;
				if (_isAsIGo==-2){
					continue;
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
	}
	return 0;
}
