/*
TODO - Remember to free the user search input and input for queue
TODO - Settings saving
*/
#define VERSION 1

#define LUAREGISTER(x,y) lua_pushcfunction(L,x);\
	lua_setglobal(L,y);
#define STARTTRACKINGGLOBALS() luaL_dostring(L,"GlobalsTrackStart()");
#define ENDTRACKINGGLOBALS() luaL_dostring(L,"GlobalsTrackRemove()");
#define CONSTANTCERTFILELOCATION "assets/curl-ca-bundle.crt"
#define CONSTANTMANGAFOLDERROOT "Manga/"
#define CONSTANTOPTIONSFOLDERROOT "Options/"
#define CONSTANTDOWNLOADERSLOCATION "assets/Downloaders/"
#define CONSTANTFONTFILE "assets/LiberationSans-Regular.ttf"
#define INPUTTYPENONE 0
#define INPUTTYPESTRING 1
#define INPUTTYPENUMBER 2
#define INPUTTYPELIST 3
#define MAXQUEUE 5
// Customizable colors confirmed?!
#define COLOROPTION 255,255,255
#define COLORSELECTED 0,255,0
#define COLORSTATUS COLOROPTION
// For lists and number input, how much you move when pressing left or right
#define LISTLEFTRIGHTJUMPOFFSET 10

#define DOWNLOAD_NONE 0
#define DOWNLOAD_CURL 1
#define DOWNLOAD_VITA 2
#define DOWNLOADTYPE DOWNLOAD_CURL

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <Lua/lua.h>
#include <Lua/lualib.h>
#include <Lua/lauxlib.h>

#include "GeneralGoodConfig.h"
#include "GeneralGoodExtended.h"
#include "GeneralGood.h"
#include "GeneralGoodGraphics.h"
#include "GeneralGoodText.h"
#include "Download.h"
#include "FpsCapper.h"
#include "KeyboardCode.h"

/*============================================================================*/
lua_State* L;
int currentTextHeight=0;
int screenHeight;
int screenWidth;
int cursorWidth;
// Fixed paths
char* mangaFolderRoot;
char* downloadersLocation;
// Queue
char* shortNameQueue[MAXQUEUE];
char* longNameQueue[MAXQUEUE];
char inputTypeQueue[MAXQUEUE];
int currentQueue=0;
// Number of times L_waitForUserInputs has been called in this script
int numberOfPrompts=0;
//===============
// OPTIONS
//===============
char downloadCoverIfPossible=1;
/*============================================================================*/
void quitApplication(){
	curl_easy_cleanup(curl_handle);
}
int calculateListOffset(int _selection, int _optionsPerScreen, int _listSize){
	int _result=0;
	if (_selection>=_optionsPerScreen/2){
		_result=_selection-_optionsPerScreen/2;
		if (_result+_optionsPerScreen>=_listSize){
			_result=_listSize-_optionsPerScreen;
		}
	}
	return _result;
}
int moveCursor(int _selection, int _listSize, char _canWrap, int _amount){
	_selection+=_amount;
	if (_selection>_listSize-1){
		if (_canWrap==1){
			_selection=0;
		}else{
			_selection=_listSize-1;
		}
	}else if (_selection<0){
		if (_canWrap==1){
			_selection=_listSize-1;
		}else{
			_selection=0;
		}
	}
	return _selection;
}
void gooditoa(int _num, char* _buffer, int _uselessBase){
	sprintf(_buffer, "%d", _num);
}
// TODO - Slowly scroll results that won't fit on the screen
// Returns -1 if user cancels
// Returns 1 based selection
int showList(char** _currentList, int _listSize, int _startingSelection){
	ControlsEnd();
	int i;
	char _optionsPerScreen;
	int _selection=_startingSelection;
	int _selectionListOffset=0;
	if (screenHeight-_listSize*currentTextHeight>0){
		_optionsPerScreen = _listSize;
	}else{
		_optionsPerScreen = screenHeight/currentTextHeight;
	}
	_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
	char* _lastUserSearchTerm=NULL;
	int _lastSearchResult=0;
	while (1){
		FpsCapStart();
		
		ControlsStart();
		if (WasJustPressed(SCE_CTRL_DOWN)){
			_selection = moveCursor(_selection,_listSize,1,1);
			_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
		}else if (WasJustPressed(SCE_CTRL_UP)){
			_selection = moveCursor(_selection,_listSize,1,-1);
			_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
		}else if (WasJustPressed(SCE_CTRL_RIGHT)){
			_selection = moveCursor(_selection,_listSize,0,LISTLEFTRIGHTJUMPOFFSET);
			_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
		}else if (WasJustPressed(SCE_CTRL_LEFT)){
			_selection = moveCursor(_selection,_listSize,0,LISTLEFTRIGHTJUMPOFFSET*-1);
			_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
		}else if (WasJustPressed(SCE_CTRL_CROSS)){
			if (_lastUserSearchTerm!=NULL){
				free(_lastUserSearchTerm);
			}
			return _selection+1;
		}else if (WasJustPressed(SCE_CTRL_CIRCLE)){
			if (_lastUserSearchTerm!=NULL){
				free(_lastUserSearchTerm);
			}
			return -1;
		}else if (WasJustPressed(SCE_CTRL_SQUARE)){
			
			// SEARCH LIST FUNCTION
			char* _tempUserAnswer = userKeyboardInput(_lastUserSearchTerm!=NULL ? _lastUserSearchTerm : "","Search",99);
			if (_lastUserSearchTerm!=NULL){
				free(_lastUserSearchTerm);
			}
			_lastUserSearchTerm = _tempUserAnswer;
			//_lastUserSearchTerm = 
			for (i=0;i<_listSize;i++){
				if (strstr(_currentList[i],_lastUserSearchTerm)!=NULL){
					_selection=i;
					_lastSearchResult=i;
					_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
					break;
				}
			}
		}else if (WasJustPressed(SCE_CTRL_TRIANGLE)){
			for (i=_lastSearchResult!=_listSize-1 ? _lastSearchResult+1 : 0;i<_listSize;i++){
				if (strstr(_currentList[i],_lastUserSearchTerm)!=NULL){
					_selection=i;
					_lastSearchResult=i;
					_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
					break;
				}
				if (i==_listSize-1){
					if (_lastSearchResult!=0){
						i=0;
						_lastSearchResult=0;
					}
				}
			}
		}
		ControlsEnd();

		StartDrawing();
		for (i=0;i<_optionsPerScreen;i++){
			GoodDrawTextColored(cursorWidth+5,i*currentTextHeight,_currentList[i+_selectionListOffset],fontSize,COLOROPTION);
		}
		GoodDrawTextColored(0,(_selection-_selectionListOffset)*currentTextHeight,">",fontSize,COLORSELECTED);
		GoodDrawTextColored(cursorWidth+5,(_selection-_selectionListOffset)*currentTextHeight,_currentList[_selection],fontSize,COLORSELECTED);
		EndDrawing();

		FpsCapWait();
	}
}
// _userInputNumber should be ONE BASED
void pushUserInput(lua_State* passedState, void* _userInputResults, int _tempType, int _userInputNumber){
	// Set a global variable to the user's choice
	// Construct variable name
	char _userInputResultName[256];
	sprintf(_userInputResultName,"userInput%02d",_userInputNumber);
	// Push different type depending on input type
	if (_tempType==INPUTTYPESTRING){
		lua_pushstring(passedState,(char*)_userInputResults);
	}else if (_tempType==INPUTTYPENUMBER || _tempType==INPUTTYPELIST){
		lua_pushnumber(passedState,*((int*)_userInputResults));
	}
	lua_setglobal(passedState, _userInputResultName);
}
int inputNumber(int _startingNumber){
	ControlsEnd();
	int _currentNumber = _startingNumber;
	char _numberTextBuffer[10];
	gooditoa(_currentNumber,_numberTextBuffer,10);
	while (1){
		FpsCapStart();

		ControlsStart();
		if (WasJustPressed(SCE_CTRL_UP)){
			_currentNumber+=1;
			gooditoa(_currentNumber,_numberTextBuffer,10);
		}else if (WasJustPressed(SCE_CTRL_DOWN)){
			_currentNumber-=1;
			if (_currentNumber<0){
				_currentNumber=0;
			}
			gooditoa(_currentNumber,_numberTextBuffer,10);
		}else if (WasJustPressed(SCE_CTRL_RIGHT)){
			_currentNumber+=LISTLEFTRIGHTJUMPOFFSET;
			gooditoa(_currentNumber,_numberTextBuffer,10);
		}else if (WasJustPressed(SCE_CTRL_LEFT)){
			_currentNumber-=LISTLEFTRIGHTJUMPOFFSET;
			if (_currentNumber<0){
				_currentNumber=0;
			}
			gooditoa(_currentNumber,_numberTextBuffer,10);
		}else if (WasJustPressed(SCE_CTRL_CROSS)){
			return _currentNumber;
		}else if (WasJustPressed(SCE_CTRL_CIRCLE)){
			return _startingNumber;
		}
		ControlsEnd();

		StartDrawing();
		GoodDrawTextColored(0,0,_numberTextBuffer,fontSize,COLORSELECTED);
		EndDrawing();

		FpsCapWait();
	}	
}
void WriteToDebugFile(const char* stuff){
	#if PLATFORM == PLAT_VITA
		FILE *fp;
		fp = fopen("ux0:data/LUAMANGAS/a.txt", "a");
		fprintf(fp,"%s\n",stuff);
		fclose(fp);
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
	#endif
}
// strcpy, but it won't copy from src to dest if the value is 1.
// You can use this to exclude certian spots
// I do not mean the ASCII character 1, which is 49.
void strcpyNO1(char* dest, const char* src){
	int i;
	int _destCopyOffset=0;
	int _srcStrlen = strlen(src);
	for (i=0;i<_srcStrlen;i++){
		if (src[i]!=1){
			dest[_destCopyOffset]=src[i];
			_destCopyOffset++;
		}
	}
}
// Same as strlen, but doesn't count any places with the value of 1 as a character.
int strlenNO1(char* src){
	int len=0;
	int i;
	for (i=0;;i++){
		if (src[i]=='\0'){
			break;
		}else if (src[i]!=1){
			len++;
		}
	}
	return len;
}
#define TEXTBOXY 0
void popupMessage(const char* _tempMsg, char _waitForAButton){
	ControlsEnd();
	// The string needs to be copied. We're going to modify it, at we can't if we just type the string into the function and let the compiler do everything else
	char message[strlen(_tempMsg)+1];
	strcpy(message,_tempMsg);
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
				break;
			}
			offsetStrlen += strlen(&message[offsetStrlen])+1;
			_numberOfLines--;
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
	return;
}
void freeQueue(int _maxQueue){
	int i;
	for (i=0;i<_maxQueue;i++){
		if (shortNameQueue[i]!=NULL){
			free(shortNameQueue[i]);
			free(longNameQueue[i]);
		}
	}
}
/*============================================================================*/
// url, filepath
int L_downloadFile(lua_State* passedState){
	const char* passedUrl = lua_tostring(passedState,1);
	const char* passedFilepath = lua_tostring(passedState,2);
	downloadToFile(passedUrl,passedFilepath);
	return 0;
}
// url
int L_downloadString(lua_State* passedState){
	const char* passedUrl = lua_tostring(passedState,1);
	char* _downloadedData;
	downloadWebpageData(passedUrl,&_downloadedData,NULL);
	lua_pushstring(passedState,_downloadedData);
	free(_downloadedData);
	return 1;
}
int L_fopen(lua_State* passedState){
	lua_pushlightuserdata(L,(void*)fopen(lua_tostring(passedState,1),lua_tostring(passedState,2)));
	return 1;
}
int L_fclose(lua_State* passedState){
	FILE* fp = (FILE*)lua_touserdata(passedState,1);
	lua_pushnumber(passedState,fclose(fp));
	return 1;
}
int L_fread(lua_State* passedState){
	FILE* fp = (FILE*)lua_touserdata(passedState,1);
	char _tempMemoryBlock[(int)lua_tonumber(passedState,2)+1];
	_tempMemoryBlock[(int)lua_tonumber(passedState,2)]='\0';
	int _readBytes = fread(&_tempMemoryBlock,lua_tonumber(passedState,2),1,fp);
	lua_pushstring(passedState,_tempMemoryBlock);
	lua_pushnumber(passedState,_readBytes);
	return 2;
}
int L_CreateDirectory(lua_State* passedState){
	createDirectory(lua_tostring(passedState,1));
	return 0;
}
// Should end with a slash
int L_getMangaFolderRoot(lua_State* passedState){
	lua_pushstring(passedState,mangaFolderRoot);
	return 1;
}
// short question, long question, type
int L_userInputQueue(lua_State* passedState){
	shortNameQueue[currentQueue] = malloc(strlen(lua_tostring(passedState,1))+1);
	longNameQueue[currentQueue] = malloc(strlen(lua_tostring(passedState,2))+1);
	inputTypeQueue[currentQueue] = lua_tonumber(passedState,3);
	strcpy((shortNameQueue[currentQueue]),lua_tostring(passedState,1));
	strcpy((longNameQueue[currentQueue]),lua_tostring(passedState,2));
	currentQueue++;
	return 0;
}
int L_shouldDownloadCovers(lua_State* passedState){
	lua_pushboolean(passedState,downloadCoverIfPossible);
	return 1;
}
int L_getVersion(lua_State* passedState){
	lua_pushnumber(passedState,VERSION);
	return 1;
}
int L_fileExists(lua_State* passedState){
	lua_pushboolean(passedState,checkFileExist(lua_tostring(passedState,1)));
	return 1;
}
//============================
// CHANGE DEPENDING ON UI MODE
//============================
// Returns false if user quit, true otherwise
int L_waitForUserInputs(lua_State* passedState){
	numberOfPrompts++;
	int i;
	int _selection=0;
	// THIS IS NOT A TRIPLE POINTER BECAUSE IT'S AN ARRAY OF DOUBLE POINTERS!
	// Array elements point to question specific list (question specific list is char** )
	// Specific list points to strings. ( char* )
	char** _listEntries[currentQueue];
	int _listEntriesLength[currentQueue];
	for (i=0;i<currentQueue;i++){
		_listEntries[i]=NULL;
	}
	void* _userInputResults[currentQueue]; // Memory is dynamiclly allocated

	char _userDidQuit=0;
	short _colonSpaceWidth=TextWidth(fontSize,": ");
	// Allocate memory for user's NUMBER AND LIST ANSWERS
	// String answers will not be allocated yet, they will be set to NULL
	// Any pointer set to NULL will not be freed
	for (i=0;i<currentQueue;i++){
		if (inputTypeQueue[i]==INPUTTYPENUMBER || inputTypeQueue[i]==INPUTTYPELIST){
			_userInputResults[i] = malloc(sizeof(int));
			*((int*)_userInputResults[i])=1;
			pushUserInput(passedState,_userInputResults[i],inputTypeQueue[i],i+1);
		}else{
			_userInputResults[i]=NULL;
		}
	}
	// Allow user to input stuff
	while (1){
		FpsCapStart();
		ControlsStart();
		if (WasJustPressed(SCE_CTRL_DOWN)){
			_selection++;
			if (_selection>currentQueue+1){
				_selection=0;
			}
		}else if (WasJustPressed(SCE_CTRL_UP)){
			_selection--;
			if (_selection<0){
				_selection=currentQueue+1;
			}
		}else if (WasJustPressed(SCE_CTRL_LEFT)){
			if (_selection<MAXQUEUE){
				if (inputTypeQueue[_selection]==INPUTTYPENUMBER){
					(*((int*)(_userInputResults[_selection])))--;
					pushUserInput(passedState,_userInputResults[_selection],inputTypeQueue[_selection],_selection+1);
				}
			}
		}else if (WasJustPressed(SCE_CTRL_RIGHT)){
			if (_selection<MAXQUEUE){
				if (inputTypeQueue[_selection]==INPUTTYPENUMBER){
					(*((int*)(_userInputResults[_selection])))++;
					pushUserInput(passedState,_userInputResults[_selection],inputTypeQueue[_selection],_selection+1);
				}
			}
		}else if (WasJustPressed(SCE_CTRL_CROSS)){
			// If they pressed the save and load button
			if (_selection==currentQueue){
				printf("save&load\n");
			}
			// If they pressed the done button
			if (_selection>=currentQueue+1){
				break;
			}
			if (inputTypeQueue[_selection]==INPUTTYPELIST){
				int _currentList=_selection;
				
				char _listFunctionName[256];
				sprintf(_listFunctionName,"InitList%02d",_currentList+1);
				if (lua_getglobal(L,_listFunctionName)==0){
					printf("Failed to get global function %s\n",_listFunctionName);
					return 0;
				}
				if (_listEntries[_currentList]==NULL){
					lua_pushboolean(L,1);
				}else{
					lua_pushboolean(L,0);
				}
				lua_call(L, 1, 1);
				// If tables return a number, it means that no table avalible. Free old table
				if (lua_type(L,-1)==LUA_TNUMBER || lua_type(L,-1)==LUA_TTABLE){
					int j;
					// Free previous list if it exists
					if (_listEntries[_currentList]!=NULL){
						for (j=0;j<_listEntriesLength[_currentList];j++){
							free(_listEntries[_currentList][j]);
						}
						free(_listEntries[_currentList]);
						_listEntries[_currentList]=NULL;
						_listEntriesLength[_currentList]=0;
					}
				}
				if (lua_type(L,-1)==LUA_TTABLE){
					int j;
					
					int _lengthOfTable = lua_rawlen(L,-1);
					_listEntriesLength[_currentList] = _lengthOfTable;
					_listEntries[_currentList] = calloc(1,sizeof(char*)*(_lengthOfTable));
					for (j=0;j<_listEntriesLength[_currentList];j++){
						lua_rawgeti(L,1,j+1); // Do j+1 because Lua is stupid
						char* _currentListEntry = (char*)lua_tostring(L,-1);
						_listEntries[_currentList][j] = calloc(1,strlen(_currentListEntry)+1);
						strcpy((_listEntries[_currentList][j]),_currentListEntry);
						lua_remove(L,-1);
					}
				}
				// Remove table or nil from stack
				lua_remove(L,-1);
				if (_listEntries[_currentList]!=NULL){
					int _tempUserAnswer = showList(_listEntries[_currentList],_listEntriesLength[_currentList],*((int*)_userInputResults[_currentList])-1); // Subtract 1 from starting index because result was one based.
					if (_tempUserAnswer!=-1){
						*((int*)(_userInputResults[_currentList]))=_tempUserAnswer;
						pushUserInput(passedState,_userInputResults[_currentList],inputTypeQueue[_currentList],_currentList+1);
					}
				}
			}else if (inputTypeQueue[_selection]==INPUTTYPENUMBER){
				*((int*)(_userInputResults[_selection])) = inputNumber(*((int*)(_userInputResults[_selection])));
				pushUserInput(passedState,_userInputResults[_selection],inputTypeQueue[_selection],_selection+1);
			}else if (inputTypeQueue[_selection]==INPUTTYPESTRING){
				_userInputResults[_selection] = userKeyboardInput(_userInputResults[_selection]!=NULL ? _userInputResults[_selection] : "",shortNameQueue[_selection],99);
				pushUserInput(passedState,_userInputResults[_selection],inputTypeQueue[_selection],_selection+1);
			}
		}else if (WasJustPressed(SCE_CTRL_CIRCLE)){
			_userDidQuit=1;
			break;
		}
		ControlsEnd();
		StartDrawing();
		GoodDrawTextColored(0,currentTextHeight*_selection,">",fontSize,COLORSELECTED);
		for (i=0;i<currentQueue;i++){
			int _currentDrawWidth=cursorWidth+5;
			GoodDrawTextColored(_currentDrawWidth,currentTextHeight*i,shortNameQueue[i],fontSize,COLOROPTION);
			_currentDrawWidth+=TextWidth(fontSize,shortNameQueue[i]);
			GoodDrawTextColored(_currentDrawWidth,currentTextHeight*i,": ",fontSize,COLOROPTION);
			_currentDrawWidth+=_colonSpaceWidth;
			if (inputTypeQueue[i]==INPUTTYPELIST){
				if (_listEntries[i]!=NULL){		
					GoodDrawTextColored(_currentDrawWidth,currentTextHeight*i,_listEntries[i][*((int*)_userInputResults[i])-1],fontSize,COLORSELECTED);
				}
			}else if (inputTypeQueue[i]==INPUTTYPENUMBER){
				char _tempNumberBuffer[10];
				gooditoa(*((int*)_userInputResults[i]),_tempNumberBuffer,10);
				GoodDrawTextColored(_currentDrawWidth,currentTextHeight*i,_tempNumberBuffer,fontSize,COLORSELECTED);
			}else if (inputTypeQueue[i]==INPUTTYPESTRING){
				if (_userInputResults[i]!=NULL){
					GoodDrawTextColored(_currentDrawWidth,currentTextHeight*i,_userInputResults[i],fontSize,COLORSELECTED);
				}
			}
		}
		GoodDrawTextColored(cursorWidth+5,currentTextHeight*i,"Save or Load Options",fontSize,COLOROPTION);
		GoodDrawTextColored(cursorWidth+5,currentTextHeight*(i+1),"Done",fontSize,COLOROPTION);
		EndDrawing();
	
		FpsCapWait();
	}

	// Free memory
	freeQueue(currentQueue);
	for (i=0;i<currentQueue;i++){
		if (_userInputResults[currentQueue]!=NULL){ // Empty string inputs will be NULL pointer
			free(_userInputResults[i]);
		}
	}
	currentQueue=0;
	if (_userDidQuit==1){
		lua_pushboolean(passedState,0);
	}else{
		lua_pushboolean(passedState,1);
	}
	return 1;
}
int L_userPopupMessage(lua_State* passedState){
	popupMessage(lua_tostring(passedState,1),1);
	return 0;
}
int L_showStatus(lua_State* passedState){
	popupMessage(lua_tostring(passedState,-1),0);
	return 0;
}
void MakeLuaUseful(){
	LUAREGISTER(L_downloadString,"downloadString");
	LUAREGISTER(L_downloadFile,"downloadFile");
	LUAREGISTER(L_fopen,"fopen");
	LUAREGISTER(L_fclose,"fclose");
	LUAREGISTER(L_fread,"fread");
	LUAREGISTER(L_CreateDirectory,"createDirectory");
	LUAREGISTER(L_getMangaFolderRoot,"rawGetMangaFolder");
	LUAREGISTER(L_userInputQueue,"userInputQueue");
	LUAREGISTER(L_waitForUserInputs,"waitForUserInputs");
	LUAREGISTER(L_shouldDownloadCovers,"shouldDownloadCovers");
	LUAREGISTER(L_getVersion,"getDownloaderVersion");
	LUAREGISTER(L_fileExists,"fileExists");
	LUAREGISTER(L_userPopupMessage,"popupMessage");
	LUAREGISTER(L_showStatus,"showStatus");
}
/*============================================================================*/
void init(){
	ClearDebugFile();
	initDownload();
	L = luaL_newstate();
	luaL_openlibs(L);
	MakeLuaUseful();

	initGraphics(640,480, &screenWidth, &screenHeight);

	// Text
	FixPath(CONSTANTFONTFILE,tempPathFixBuffer,TYPE_EMBEDDED);
	LoadFont(tempPathFixBuffer);
	currentTextHeight = TextHeight(fontSize);
	cursorWidth = TextWidth(fontSize,">");

	// Make data folder
	FixPath("",tempPathFixBuffer,TYPE_DATA);
	createDirectory(tempPathFixBuffer);

	FixPath(CONSTANTDOWNLOADERSLOCATION,tempPathFixBuffer,TYPE_EMBEDDED);
	downloadersLocation = malloc(strlen(tempPathFixBuffer)+1);
	strcpy(downloadersLocation,tempPathFixBuffer);
	FixPath(CONSTANTMANGAFOLDERROOT,tempPathFixBuffer,TYPE_DATA);
	mangaFolderRoot = malloc(strlen(tempPathFixBuffer)+1);
	strcpy(mangaFolderRoot,tempPathFixBuffer);

	createDirectory(mangaFolderRoot);

	FixPath("assets/Init.lua",tempPathFixBuffer,TYPE_EMBEDDED);
	luaL_dofile(L,tempPathFixBuffer);
	FixPath("assets/GlobalTracking.lua",tempPathFixBuffer,TYPE_EMBEDDED);
	luaL_dofile(L,tempPathFixBuffer);
}
void resetScriptData(){
	int i;
	for (i=0;i<MAXQUEUE;i++){
		inputTypeQueue[i]=INPUTTYPENONE;
	}
	freeQueue(MAXQUEUE);
	currentQueue=0;
	numberOfPrompts=0;
}
void doScript(char* luaFileToUse){
	resetScriptData();
	// We do the cleanup for you!
	STARTTRACKINGGLOBALS();

	char luaFilenameComplete[strlen(luaFileToUse)+strlen(downloadersLocation)+5];
	strcpy(luaFilenameComplete,downloadersLocation);
	strcat(luaFilenameComplete,luaFileToUse);
	strcat(luaFilenameComplete,".lua");

	char luaDownloadFunctionCallComplete[strlen(luaFileToUse)+strlen("Download();")+1];
	strcpy(luaDownloadFunctionCallComplete,luaFileToUse);
	strcat(luaDownloadFunctionCallComplete,"Download();");

	printf("Loading LUA %s\n",luaFilenameComplete);
	luaL_dofile(L,luaFilenameComplete);
	printf("Running download function %s\n",luaDownloadFunctionCallComplete);
	luaL_dostring(L,luaDownloadFunctionCallComplete);

	// Clean the leftovers
	ENDTRACKINGGLOBALS();
}
int main(int argc, char *argv[]){
	init();

	doScript("MangaReader");	

	// End
	quitApplication();
	return 0;
}
