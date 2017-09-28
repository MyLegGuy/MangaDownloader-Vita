#define VERSION 1

#define CERTFILELOCATION "./curl-ca-bundle.crt"
#define LUAREGISTER(x,y) lua_pushcfunction(L,x);\
	lua_setglobal(L,y);
#define STARTTRACKINGGLOBALS() luaL_dostring(L,"GlobalsTrackStart()");
#define ENDTRACKINGGLOBALS() luaL_dostring(L,"GlobalsTrackRemove()");
#define MANGAFOLDERROOT "./Manga/"
#define DOWNLOADERSLOCATION "./Downloaders/"
#define INPUTTYPESTRING 1
#define INPUTTYPENUMBER 2
#define INPUTTYPELIST 3

#define COLOROPTION 255,255,255
#define COLORSELECTED 0,255,0

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

/*============================================================================*/
lua_State* L;
char* shortNameQueue[5];
char* longNameQueue[5];
char inputTypeQueue[5];
int currentQueue=0;
int currentTextHeight=0;

int screenHeight;
int screenWidth;
int cursorWidth;
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
	lua_pushstring(passedState,MANGAFOLDERROOT);
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
// TODO - Slowly scroll results that won't fit on the screen
// Returns -1 if user cancels
// Returns 1 based selection
int showList(char** _currentList, int _listSize, int _startingSelection){
	ControlsEnd();
	int i;
	int j;
	char _optionsPerScreen;
	int _selection=_startingSelection;
	int _selectionListOffset=0;
	char _listHeightIsMax;
	if (screenHeight-_listSize*currentTextHeight>0){
		_optionsPerScreen = _listSize;
		_listHeightIsMax=1;
	}else{
		_optionsPerScreen = screenHeight/currentTextHeight;
		_listHeightIsMax=0;
	}
	_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
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
			return _selection+1;
		}else if (WasJustPressed(SCE_CTRL_CIRCLE)){
			return -1;
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
		printf("pushed %d\n",*((int*)_userInputResults));
	}
	printf("var name is %s\n",_userInputResultName);
	lua_setglobal(passedState, _userInputResultName);
}
int L_waitForUserInputs(lua_State* passedState){
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

	// Allocate memory for user's NUMBER AND LIST ANSWERS
	// String answers will not be allocated yet, they will be set to NULL
	// Any pointer set to NULL will not be freed
	for (i=0;i<currentQueue;i++){
		if (inputTypeQueue[i]==INPUTTYPENUMBER || inputTypeQueue[i]==INPUTTYPELIST){
			_userInputResults[i] = malloc(sizeof(int));
			*((int*)_userInputResults[i])=1;
		}
	}

	// Allow user to input stuff
	while (1){
		FpsCapStart();
		ControlsStart();
		if (WasJustPressed(SCE_CTRL_DOWN)){
			_selection++;
			if (_selection>currentQueue){
				_selection=0;
			}
		}else if (WasJustPressed(SCE_CTRL_UP)){
			_selection--;
			if (_selection<0){
				_selection=currentQueue;
			}
		}else if (WasJustPressed(SCE_CTRL_CROSS)){
			// If they pressed the done button
			if (_selection>=currentQueue){
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
				if (lua_type(L,-1)==LUA_TTABLE){
					int j;
					// Free previous list if it exists
					if (_listEntries[_currentList]!=NULL){
						for (j=0;j<_listEntriesLength[_currentList];j++){
							free(_listEntries[_currentList][j]);
						}
						free(_listEntries[_currentList]);
						_listEntries[_currentList]=NULL;
					}
					
					int _lengthOfTable = lua_rawlen(L,-1);
					_listEntriesLength[_currentList] = _lengthOfTable;
					_listEntries[_currentList] = malloc(sizeof(char*)*(_lengthOfTable));
					for (j=0;j<_listEntriesLength[_currentList];j++){
						lua_rawgeti(L,1,j+1); // Do j+1 because Lua is stupid
						char* _currentListEntry = (char*)lua_tostring(L,-1);
						_listEntries[_currentList][j] = malloc(strlen(_currentListEntry)+1);
						strcpy((_listEntries[_currentList][j]),_currentListEntry);
						lua_remove(L,-1);
					}
				}
				// Remove table or nil from stack
				lua_remove(L,-1);
				if (_listEntries[_currentList]!=NULL){
					printf("Here, I can show the table.\n");
					//for (i=0;i<_listEntriesLength[_currentList];i++){
					//	printf("Entry %d: %s\n",i,_listEntries[_currentList][i]);
					//}
					int _tempUserAnswer = showList(_listEntries[_currentList],_listEntriesLength[_currentList],*((int*)_userInputResults[_currentList])-1); // Subtract 1 from starting index because result was one based.
					if (_tempUserAnswer!=-1){
						*((int*)(_userInputResults[_currentList]))=_tempUserAnswer;
						printf("answer is %d\n",*((int*)(_userInputResults[_currentList])));
						pushUserInput(passedState,_userInputResults[_currentList],inputTypeQueue[_currentList],_currentList+1);
					}else{
						printf("user did not answer.");
					}
				}else{
					// TODO - Tell the user that this is currently an empty list.
				}
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
			if (_listEntries[i]!=NULL){
				_currentDrawWidth+=TextWidth(fontSize,shortNameQueue[i]);
				GoodDrawTextColored(_currentDrawWidth,currentTextHeight*i,": ",fontSize,COLOROPTION);
				_currentDrawWidth+=TextWidth(fontSize,": ");
				GoodDrawTextColored(_currentDrawWidth,currentTextHeight*i,_listEntries[i][*((int*)_userInputResults[i])-1],fontSize,COLORSELECTED);
			}
		}
		GoodDrawTextColored(cursorWidth+5,currentTextHeight*i,"Done",fontSize,COLOROPTION);
		EndDrawing();
	
		FpsCapWait();
	}

	// Free memory
	for (i=0;i<currentQueue;i++){
		free(shortNameQueue[i]);
		free(longNameQueue[i]);
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
/*
// Returns false if user quit, true otherwise
int L_waitForUserInputs(lua_State* passedState){
	int i;
	void* userInputResults[currentQueue]; // Memory is dynamiclly allocated
	char lastTypedLine[256];
	char _userDidQuit=0;
	for (i=0;i<currentQueue;i++){
		// Print info
		printf("===\n%s\n===\n%s\n",shortNameQueue[i],longNameQueue[i]);
		// Specific to type of input
		if (inputTypeQueue[i]==INPUTTYPENUMBER){
			userInputResults[i] = malloc(sizeof(int));
			scanf("%d",(int*)userInputResults[i]);
		}else if (inputTypeQueue[i]==INPUTTYPESTRING){
			scanf("%[^\n]",lastTypedLine);
			userInputResults[i] = malloc(strlen(lastTypedLine)+1);
			strcpy(userInputResults[i],lastTypedLine);
		}else if (inputTypeQueue[i]==INPUTTYPELIST){
			printf("===\n");

			char _listFunctionName[256];
			sprintf(_listFunctionName,"InitList%02d",i+1);
			if (lua_getglobal(L,_listFunctionName)==0){
				printf("Failed to get global function %s\n",_listFunctionName);
				return 0;
			}
			// Pass isFirstTime to function
			lua_pushboolean(L,1);
			lua_call(L, 1, 1);

			if (lua_type(L,-1)==LUA_TTABLE){
				int j;
				int _lengthOfTable = lua_rawlen(L,-1);
				for (j=1;j<=_lengthOfTable;j++){
					lua_rawgeti(L,1,j);
					printf("%d: %s\n",j,lua_tostring(L,-1));
					lua_remove(L,-1);
				}
			}else{
				printf("DID NOT REUTRN TABLE!");
				return 0;
			}
			// Remove table or nil from stack
			lua_remove(L,-1);
			printf("===\n");
			// Copy of code from string type input
			printf("Entry from list you want?\n");
			userInputResults[i] = malloc(sizeof(int));
			scanf("%d",(int*)userInputResults[i]);
		}


		// Set a global variable to the user's choice
		// Construct variable name
		char _userInputResultName[256];
		sprintf(_userInputResultName,"userInput%02d",i+1);
		// Push different type depending on input type
		if (inputTypeQueue[i]==INPUTTYPESTRING){
			lua_pushstring(passedState,(char*)userInputResults[i]);
		}else if (inputTypeQueue[i]==INPUTTYPENUMBER || inputTypeQueue[i]==INPUTTYPELIST){
			lua_pushnumber(passedState,*((int*)userInputResults[i]));
		}
		lua_setglobal(L, _userInputResultName);
	}

	// Free memory
	for (i=0;i<currentQueue;i++){
		free(shortNameQueue[currentQueue]);
		free(longNameQueue[currentQueue]);
		free(userInputResults[currentQueue]);
	}
	currentQueue=0;

	if (_userDidQuit==1){
		lua_pushboolean(passedState,0);
	}else{
		lua_pushboolean(passedState,1);
	}
	return 1;
}*/
int L_userPopupMessage(lua_State* passedState){
	printf(lua_tostring(passedState,1));
	printf("\n*** Press any key to continue... ***");
	getchar();
	return 0;
}
void MakeLuaUseful(){
	LUAREGISTER(L_downloadString,"downloadString");
	LUAREGISTER(L_downloadFile,"downloadFile");
	LUAREGISTER(L_fopen,"fopen"); // TODO - Following (3) are untested. Also make functions for rest of IO stuff
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
}
/*============================================================================*/
void init(){
	#if DOWNLOADTYPE == DOWNLOAD_CURL
		initCurl();
	#endif
	L = luaL_newstate();
	luaL_openlibs(L);
	MakeLuaUseful();

	initGraphics(640,480, &screenWidth, &screenHeight);

	// Text
	LoadFont("./LiberationSans-Regular.ttf");
	currentTextHeight = TextHeight(fontSize);
	cursorWidth = TextWidth(fontSize,">");
}
int main(int argc, char *argv[]){
	init();
	//while (1){
	//	StartDrawing();
	//	GoodDrawTextColored(0,0,"noob",fontSize,255,0,0);
	//	EndDrawing();
	//	ControlsStart();
	//	ControlsEnd();
	//}
	createDirectory(MANGAFOLDERROOT);
	// We do the cleanup for you!
	luaL_dofile(L,"./Init.lua");
	luaL_dofile(L,"./GlobalTracking.lua");
	lua_pushstring(L,MANGAFOLDERROOT);
	lua_setglobal(L,"downloadFolder");
	STARTTRACKINGGLOBALS();

	char* luaFileToUse = "DynastyScans";
	
	char luaFilenameComplete[strlen(luaFileToUse)+strlen(DOWNLOADERSLOCATION)+5];
	strcpy(luaFilenameComplete,DOWNLOADERSLOCATION);
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

	// End
	quitApplication();
	return 0;
}
/*
TOOD - Status. Tell the user what's going on.
	I imagine this as a bar taking up the bottom 1/3 of the screen
	It can also display the description of the option
*/