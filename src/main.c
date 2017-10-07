/*
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
#define INPUTTYPELISTMULTI 4
#define MAXQUEUE 5
#define SCROLLCHARSPEED 10
// Customizable colors confirmed?!
#define COLOROPTION 255,255,255
#define COLORMARKED 255,100,0
#define COLORMARKEDHOVER 255,190,0
#define COLORSELECTED 0,255,0
#define COLORSTATUS COLOROPTION
#define COLORINVALID 255,0,0
#define COLORVALID 0,255,0
#define COLORMAYBE 255,255,0
// For lists and number input, how much you move when pressing left or right
#define LISTLEFTRIGHTJUMPOFFSET 10
// Max number of downloader scripts
#define MAXFILES 20

#define DOWNLOAD_NONE 0
#define DOWNLOAD_CURL 1
#define DOWNLOAD_VITA 2
#define DOWNLOADTYPE DOWNLOAD_CURL

#define SCROLLSTATUS_NOSCROLL 0
#define SCROLLSTATUS_SCROLLING 1
#define SCROLLSTATUS_WAITING 2
#define SCROLLSTATUS_NEEDCHECK 3
#define SCROLLSTATUS_ENDWAITPLUSONE 4
#define SCROLLSTATUS_ENDWAIT 5

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <Lua/lua.h>
#include <Lua/lualib.h>
#include <Lua/lauxlib.h>

// main.h
	void WriteToDebugFile(const char* stuff);
	void WriteIntToDebugFile(int a);
	char popupMessage(const char* _tempMsg, char _waitForAButton, char _isQuestion);

#include "GeneralGoodConfig.h"
#include "GeneralGoodExtended.h"
#include "GeneralGood.h"
#include "GeneralGoodGraphics.h"
#include "GeneralGoodText.h"
#include "Download.h"
#include "FpsCapper.h"
#include "KeyboardCode.h"
#include "OpenBSDstrcharstr.h"
#include "LinkedList.h"

/*============================================================================*/
lua_State* L;
int currentTextHeight=0;
int screenHeight;
int screenWidth;
int cursorWidth;
// Fixed paths
char* mangaFolderRoot;
char* downloadersLocation;
char* optionsLocation;
// Queue
char* shortNameQueue[MAXQUEUE];
char* longNameQueue[MAXQUEUE];
char inputTypeQueue[MAXQUEUE];
void* userInputResults[MAXQUEUE]; // Memory is dynamiclly allocated
int currentQueue=0;
// Number of times L_waitForUserInputs has been called in this script
int numberOfPrompts=0;
// Taken from a local variable.
// MAY NOT EXIST AFTER A SCRIPT FINISHES
char* currentScriptName;
//===============
// OPTIONS
//===============
char downloadCoverIfPossible=1;
/*============================================================================*/
void nothingFunction(){
}
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
// Returns -1 if user cancels
// Returns 1 based selection
int showList(char** _currentList, int _listSize, int _startingSelection, NathanLinkedList* _multiList){
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
	char _selectedList[_multiList!=NULL ? _listSize : 1];
	if (_multiList!=NULL){
		for (i=0;i<_listSize;i++){
			_selectedList[i]=0;
		}
		int _cachedListLength = getLinkedListLength(_multiList);
		// Length of the list is 1 based, so using less than is okay.
		for (i=0;i<_cachedListLength;i++){
			NathanLinkedList* _lastGet = getLinkedList(_multiList,i+1);
			if (_lastGet->memory!=NULL){
				// Is one based, so minus one.
				_selectedList[(int)*(_lastGet->memory)-1]=1;
			}
		}
	}
	signed int _valueToReturn=-1;
	int _framesUntilScroll=60;
	int _scrollCharOffset=0;
	char _scrollStatus=SCROLLSTATUS_NEEDCHECK;
	while (1){
		FpsCapStart();
		ControlsStart();
		if (WasJustPressed(SCE_CTRL_DOWN)){
			_selection = moveCursor(_selection,_listSize,1,1);
			_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
			_scrollStatus = SCROLLSTATUS_NEEDCHECK;
		}else if (WasJustPressed(SCE_CTRL_UP)){
			_selection = moveCursor(_selection,_listSize,1,-1);
			_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
			_scrollStatus = SCROLLSTATUS_NEEDCHECK;
		}else if (WasJustPressed(SCE_CTRL_RIGHT)){
			_selection = moveCursor(_selection,_listSize,0,LISTLEFTRIGHTJUMPOFFSET);
			_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
			_scrollStatus = SCROLLSTATUS_NEEDCHECK;
		}else if (WasJustPressed(SCE_CTRL_LEFT)){
			_selection = moveCursor(_selection,_listSize,0,LISTLEFTRIGHTJUMPOFFSET*-1);
			_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
			_scrollStatus = SCROLLSTATUS_NEEDCHECK;
		}else if (WasJustPressed(SCE_CTRL_CROSS)){
			if (_multiList==NULL){
				_valueToReturn = _selection+1;
				break;
			}else{
				if (_selectedList[_selection]==0){
					_selectedList[_selection]=1;
				}else if (_selectedList[_selection]==1){
					_selectedList[_selection]=0;
				}
			}
		}else if (WasJustPressed(SCE_CTRL_CIRCLE)){
			if (_multiList!=NULL){
				if (popupMessage("Are you sure you would like to go back? You will loose all your selected entries. (If you want to keep your selected entries, answer \"no\" to this question then press START.)",1,1)==1){
					_valueToReturn = -1;
					break;
				}
			}else{
				_valueToReturn = -1;
				break;
			}
		}else if (WasJustPressed(SCE_CTRL_SQUARE)){
			// SEARCH LIST FUNCTION
			char* _tempUserAnswer = userKeyboardInput(_lastUserSearchTerm!=NULL ? _lastUserSearchTerm : "","Search",99);
			if (_lastUserSearchTerm!=NULL){
				free(_lastUserSearchTerm);
				_lastUserSearchTerm=NULL;
			}
			if (_tempUserAnswer!=NULL){
				_lastUserSearchTerm = _tempUserAnswer;
				//_lastUserSearchTerm = 
				for (i=0;i<_listSize;i++){
					if (strcasestr(_currentList[i],_lastUserSearchTerm)!=NULL){
						_selection=i;
						_lastSearchResult=i;
						_selectionListOffset = calculateListOffset(_selection,_optionsPerScreen,_listSize);
						break;
					}
				}
				_scrollStatus = SCROLLSTATUS_NEEDCHECK;
			}
		}else if (WasJustPressed(SCE_CTRL_TRIANGLE)){
			for (i=_lastSearchResult!=_listSize-1 ? _lastSearchResult+1 : 0;i<_listSize;i++){
				if (strcasestr(_currentList[i],_lastUserSearchTerm)!=NULL){
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
			_scrollStatus = SCROLLSTATUS_NEEDCHECK;
		}else if (WasJustPressed(SCE_CTRL_START)){
			if (_multiList!=NULL){
				freeLinkedList(_multiList);
				_multiList = calloc(1,sizeof(NathanLinkedList));
				for (i=0;i<_listSize;i++){
					if (_selectedList[i]==1){
						NathanLinkedList* _lastAddedEntry = addToLinkedList(_multiList);
						_lastAddedEntry->memory = calloc(1,sizeof(int*));
						*(_lastAddedEntry->memory)=i+1;
					}
				}
				_valueToReturn = (int)(_multiList);
				break;
			}
		}
		ControlsEnd();

		StartDrawing();
		for (i=0;i<_optionsPerScreen;i++){
			if (i+_selectionListOffset!=_selection){
				if (_multiList==NULL){
					GoodDrawTextColored(cursorWidth+5,i*currentTextHeight,_currentList[i+_selectionListOffset],fontSize,COLOROPTION);
				}else{
					if (_selectedList[i+_selectionListOffset]==1){
						GoodDrawTextColored(cursorWidth+5,i*currentTextHeight,_currentList[i+_selectionListOffset],fontSize,COLORMARKED);
					}else{
						GoodDrawTextColored(cursorWidth+5,i*currentTextHeight,_currentList[i+_selectionListOffset],fontSize,COLOROPTION);
					}
				}
			}
		}
		if (_multiList==NULL){
			if (_scrollStatus!=SCROLLSTATUS_NEEDCHECK){
				GoodDrawTextColored(cursorWidth+5,(_selection-_selectionListOffset)*currentTextHeight,&(_currentList[_selection][_scrollCharOffset]),fontSize,COLORSELECTED);
			}else{
				GoodDrawTextColored(cursorWidth+5,(_selection-_selectionListOffset)*currentTextHeight,(_currentList[_selection]),fontSize,COLORSELECTED);
			}
		}else{
			if (_scrollStatus!=SCROLLSTATUS_NEEDCHECK){
				if (_selectedList[_selection]==1){
					GoodDrawTextColored(cursorWidth+5,(_selection-_selectionListOffset)*currentTextHeight,&(_currentList[_selection][_scrollCharOffset]),fontSize,COLORMARKEDHOVER);
				}else{
					GoodDrawTextColored(cursorWidth+5,(_selection-_selectionListOffset)*currentTextHeight,&(_currentList[_selection][_scrollCharOffset]),fontSize,COLORSELECTED);
				}
			}else{
				if (_selectedList[_selection]==1){
					GoodDrawTextColored(cursorWidth+5,(_selection-_selectionListOffset)*currentTextHeight,(_currentList[_selection]),fontSize,COLORMARKEDHOVER );
				}else{
					GoodDrawTextColored(cursorWidth+5,(_selection-_selectionListOffset)*currentTextHeight,(_currentList[_selection]),fontSize,COLORSELECTED);
				}
				
			}
		}
		GoodDrawTextColored(0,(_selection-_selectionListOffset)*currentTextHeight,">",fontSize,COLORSELECTED);
		EndDrawing();

		// Timer until long words start to scroll.
		if (_scrollStatus!=SCROLLSTATUS_NOSCROLL){
			if (_scrollStatus==SCROLLSTATUS_NEEDCHECK){
				if (TextWidth(fontSize,_currentList[_selection])+cursorWidth+5>screenWidth){
					_scrollStatus = SCROLLSTATUS_WAITING;	
				}
				_framesUntilScroll=60;
				_scrollCharOffset=0;
			}else if (_scrollStatus==SCROLLSTATUS_WAITING){
				_framesUntilScroll--;
				if (_framesUntilScroll==0){
					_scrollStatus = SCROLLSTATUS_SCROLLING;
					_framesUntilScroll=SCROLLCHARSPEED;
				}
			}else if (_scrollStatus==SCROLLSTATUS_SCROLLING){
				_framesUntilScroll--;
				if (_framesUntilScroll==0){
					_framesUntilScroll = SCROLLCHARSPEED;
					_scrollCharOffset++;
					if (TextWidth(fontSize,&(_currentList[_selection][_scrollCharOffset]))+cursorWidth+5<screenWidth){
						_scrollStatus = SCROLLSTATUS_ENDWAITPLUSONE;
						_framesUntilScroll=SCROLLCHARSPEED;
					}
				}
			}else if (_scrollStatus==SCROLLSTATUS_ENDWAIT){
				_framesUntilScroll--;
				if (_framesUntilScroll<=0){
					_scrollStatus=SCROLLSTATUS_WAITING;
					_framesUntilScroll=60;
					_scrollCharOffset=0;
				}
			}else if (_scrollStatus==SCROLLSTATUS_ENDWAITPLUSONE){
				_framesUntilScroll--;
				if (_framesUntilScroll==0){
					_scrollCharOffset++;
					_scrollStatus = SCROLLSTATUS_ENDWAIT;
					_framesUntilScroll=60;
				}
			}
		}
		FpsCapWait();
	}
	if (_lastUserSearchTerm!=NULL){
		free(_lastUserSearchTerm);
	}
	ControlsEnd();
	return _valueToReturn;
}
// _userInputNumber should be ONE BASED
char* getUserInputResultName(int _userInputNumber){
	char* _userInputResultName = malloc(strlen("userInput")+2+1);
	sprintf(_userInputResultName,"userInput%02d",_userInputNumber);
	return _userInputResultName;
}
// _userInputNumber should be ONE BASED
// If you're pushing INPUTTYPELISTMULTI, make sure the table is at the top of the stack already.
void pushUserInput(lua_State* passedState, void* _userInputResults, int _tempType, int _userInputNumber){
	char* _userInputResultName = getUserInputResultName(_userInputNumber);
	// Push different type depending on input type
	if (_tempType==INPUTTYPESTRING){
		lua_pushstring(passedState,(char*)_userInputResults);
	}else if (_tempType==INPUTTYPENUMBER || _tempType==INPUTTYPELIST){
		lua_pushnumber(passedState,*((int*)_userInputResults));
	}else if (_tempType==INPUTTYPELISTMULTI){
		// Do nothing, list is already ready.
	}
	lua_setglobal(passedState, _userInputResultName);
	free(_userInputResultName);
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
void freeQueue(int _maxQueue){
	int i;
	for (i=0;i<_maxQueue;i++){
		if (shortNameQueue[i]!=NULL){
			free(shortNameQueue[i]);
			free(longNameQueue[i]);
		}
	}
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
char* getOptionsFileLocation(int _slot, int _specificOptionsNumber){
	char* _compiledOptionsPath = malloc(strlen(optionsLocation)+strlen(currentScriptName)+2+1+3+1); // Here lies 2 hours of my time. I forgot about the dash, didn't add a byte for that. RIP.
	sprintf(_compiledOptionsPath,"%s%s%02d-%03d",optionsLocation,currentScriptName,_specificOptionsNumber,_slot);
	return _compiledOptionsPath;
}
// _listNumber should be 1 based
// Returns 1 if worked
// Adds one to the stack
char callListInit(lua_State* passedState, char _listNumber, char _firstTime){
	char _listFunctionName[12];
	sprintf(_listFunctionName,"InitList%02d",_listNumber);
	if (lua_getglobal(passedState,_listFunctionName)!=LUA_TFUNCTION){
		WriteToDebugFile("Failed to get global function");
		WriteToDebugFile(_listFunctionName);
		return 0;
	}
	lua_pushnumber(passedState,_firstTime);
	lua_call(passedState, 1, 1);
	return 1;
}
// _listNumber should be 1 based
char callListFinish(lua_State* passedState, char _listNumber){
	char _listFunctionName[11];
	sprintf(_listFunctionName,"EndList%02d",_listNumber);
	if (lua_getglobal(passedState,_listFunctionName)==LUA_TFUNCTION){
		lua_call(passedState, 0, 0);
	}
	lua_pop(passedState,1);
	return 1;
}
// Removes one from the stack
int assignAfterListInit(lua_State* passedState, char*** _listEntries, int _previousLength, int _tableIndex){
	// If tables return a number, it means that no table avalible. Free old table
	if (lua_type(passedState,_tableIndex)==LUA_TNUMBER || lua_type(passedState,_tableIndex)==LUA_TTABLE){
		int j;
		// Free previous list if it exists
		if ((*_listEntries)!=NULL){
			for (j=0;j<_previousLength;j++){
				free((*_listEntries)[j]);
			}
			free((*_listEntries));
			(*_listEntries)=NULL;
		}
	}
	int _lengthOfTable=_previousLength;
	if (lua_type(passedState,_tableIndex)==LUA_TTABLE){
		int j;
		_lengthOfTable = lua_rawlen(passedState,_tableIndex);
		//_listEntriesLength[_currentList] = _lengthOfTable;
		*_listEntries = calloc(1,sizeof(char*)*(_lengthOfTable));
		for (j=0;j<_lengthOfTable;j++){
			lua_rawgeti(passedState,_tableIndex,j+1); // Do j+1 because Lua is stupid
			char* _currentListEntry = (char*)lua_tostring(passedState,-1);
			(*_listEntries)[j] = calloc(1,strlen(_currentListEntry)+1);
			strcpy(((*_listEntries)[j]),_currentListEntry);
			lua_remove(passedState,-1);
		}
	}
	if (_tableIndex==-1){
		// Remove table or nil from stack
		lua_remove(passedState,-1);
	}
	return _lengthOfTable;
}
void doScript(char* luaFileToUse){
	resetScriptData();
	currentScriptName=luaFileToUse;
	// We do the cleanup for you!
	STARTTRACKINGGLOBALS();
	char luaFilenameComplete[strlen(luaFileToUse)+strlen(downloadersLocation)+5];
	strcpy(luaFilenameComplete,downloadersLocation);
	strcat(luaFilenameComplete,luaFileToUse);
	if (luaL_dofile(L,luaFilenameComplete)!=0){
		popupMessage("Failed to run Lua file.",0,0);
	}
	if (lua_getglobal(L,"MyLegGuy_Download")==0){
		popupMessage("Could not find MyLegGuy_Download function.",1,0);
		lua_pop(L,1);
		return;
	}else{
		lua_call(L,0,0);
	}
	// Clean the leftovers
	ENDTRACKINGGLOBALS();
}
// Prompt the user to choose a script file from the proper script directory. Returned string is malloc'd
char* chooseScript(){
	// Select download script
	CROSSDIR dir;
	CROSSDIRSTORAGE lastStorage;
	dir = openDirectory (downloadersLocation);
	if (dirOpenWorked(dir)==0){
		popupMessage("Script directory missing! It should've been included in the VPK. So....MyLegGuy probably forgot to include it with the VPK. You should go give him a heads up. Pressing X will show you the path of the folder that is supposed to exist.",1,0);
		popupMessage(downloadersLocation,1,0);
		return NULL;
	}
	char* _filenames[MAXFILES]={NULL};
	int i;
	for (i=0;i<MAXFILES;i++){
		if (directoryRead(&dir,&lastStorage) == 0){
			break;
		}
		_filenames[i] = malloc(strlen(getDirectoryResultName(&lastStorage))+1);
		strcpy(_filenames[i],getDirectoryResultName(&lastStorage));
	}
	directoryClose (dir);
	int _userChosenFileIndex = showList(_filenames,i,0,NULL);
	if (_userChosenFileIndex==-1){
		return NULL;
	}
	for (i=0;_filenames[i]!=NULL;i++){
		if (i+1!=_userChosenFileIndex){
			free(_filenames[i]);
		}
	}
	return _filenames[_userChosenFileIndex-1];
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
	lua_pushlightuserdata(passedState,(void*)fopen(lua_tostring(passedState,1),lua_tostring(passedState,2)));
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
// Slot, value
int L_setUserInput(lua_State* passedState){
	int _slot = lua_tonumber(passedState,1)-1;
	if (inputTypeQueue[_slot]==INPUTTYPELISTMULTI){
		freeLinkedList(userInputResults[_slot]);
		userInputResults[_slot] = calloc(1,sizeof(NathanLinkedList));
		lua_createtable(passedState, 0, 0);
		pushUserInput(passedState,NULL,INPUTTYPELISTMULTI,_slot+1);
		return 0;
	}
	if (lua_type(passedState,2)==LUA_TNUMBER){
		*((int*)userInputResults[_slot])=lua_tonumber(passedState,2);
	}else if (lua_type(passedState,2)==LUA_TSTRING){
		const char* _toSetString = lua_tostring(passedState,2);
		(userInputResults[_slot])=malloc(strlen(_toSetString)+1);
		strcpy(userInputResults[_slot],_toSetString);
	}else{
		printf("Unknwon set type. No data set\n");
	}
	pushUserInput(passedState,userInputResults[_slot],inputTypeQueue[_slot],_slot+1);
	return 0;
}
int L_assignListData(lua_State* passedState){
	char*** _lists = ((char***)lua_touserdata(passedState,1));
	int* _listLengths = ((int*)lua_touserdata(passedState,2));
	int _listToChange = lua_tonumber(passedState,3);
	_listLengths[_listToChange] = assignAfterListInit(passedState,&(_lists[_listToChange]),_listLengths[_listToChange],4);
	return 0;
}
int L_printListStuff(lua_State* passedState){
	char*** _lists = ((char***)lua_touserdata(passedState,1));
	int* _listLengths = ((int*)lua_touserdata(passedState,2));
	printf("We tryna print");
	printf("Adress of legnths %p\n",&_listLengths);
	if (_listLengths==NULL){
		printf("is null.\n");
	}
	printf("First length is %s\n",_lists[0][0]);
	printf("First length is %d\n",_listLengths[0]);
	return 0;
}
// Returns false if user quit, true otherwise
int L_waitForUserInputs(lua_State* passedState){
	numberOfPrompts++;
	char _specificOptionsNumber=255;
	char _saveAndLoadEnabled=0;
	if (lua_type(passedState,1)==LUA_TNUMBER){
		_specificOptionsNumber = lua_tonumber(passedState,1);
		_saveAndLoadEnabled=1;
	}
	int i;
	int _selection=0;
	// THIS IS NOT A TRIPLE POINTER BECAUSE IT'S AN ARRAY OF DOUBLE POINTERS!
	// Array elements point to question specific list (question specific list is char** )
	// Specific list points to strings. ( char* )
	char** _listEntries[currentQueue];
	int _listEntriesLength[currentQueue];
	for (i=0;i<currentQueue;i++){
		_listEntriesLength[i]=0;
		_listEntries[i]=NULL;
	}
	// Give the list to Lua for use with Lua functions for lists.
	lua_pushlightuserdata(passedState,_listEntries);
	lua_setglobal(passedState,"currentQueueCLists");
	lua_pushlightuserdata(passedState,_listEntriesLength);
	lua_setglobal(passedState,"currentQueueCListsLength");

	unsigned char _saveOrLoadSlot=0;
	char _userDidQuit=0;
	short _colonSpaceWidth=TextWidth(fontSize,": ");
	// - slot 1]
	char _slotString[13] = " - slot 0]";
	char _currentSlotExists;
	char* _compiledOptionsPath = getOptionsFileLocation(_saveOrLoadSlot,_specificOptionsNumber);
	_currentSlotExists = checkFileExist(_compiledOptionsPath);

	// Allocate memory for user's NUMBER AND LIST ANSWERS
	// String answers will not be allocated yet, they will be set to NULL
	// Any pointer set to NULL will not be freed
	for (i=0;i<currentQueue;i++){
		if (inputTypeQueue[i]==INPUTTYPENUMBER || inputTypeQueue[i]==INPUTTYPELIST){
			userInputResults[i] = malloc(sizeof(int));
			*((int*)userInputResults[i])=1;
			pushUserInput(passedState,userInputResults[i],inputTypeQueue[i],i+1);
		}else if (inputTypeQueue[i]==INPUTTYPELISTMULTI){
			// Must be calloc because we check if a pointer in this struct is NULL.
			userInputResults[i] = calloc(1,sizeof(NathanLinkedList));
		}else{
			userInputResults[i]=NULL;
		}
	}
	// Allow user to input stuff
	while (1){
		FpsCapStart();
		ControlsStart();
		if (WasJustPressed(SCE_CTRL_DOWN)){
			_selection++;
			if ((_saveAndLoadEnabled==1 && _selection>currentQueue+2) || (_saveAndLoadEnabled==0 && _selection>currentQueue)){
				_selection=0;
			}
		}else if (WasJustPressed(SCE_CTRL_UP)){
			_selection--;
			if (_selection<0){
				if (_saveAndLoadEnabled==1){
					_selection=currentQueue+2;
				}else{
					_selection=currentQueue;
				}
			}
		}else if (WasJustPressed(SCE_CTRL_LEFT)){
			if (_selection<currentQueue){
				if (inputTypeQueue[_selection]==INPUTTYPENUMBER){
					(*((int*)(userInputResults[_selection])))--;
					pushUserInput(passedState,userInputResults[_selection],inputTypeQueue[_selection],_selection+1);
				}
			}else{
				if (_selection==currentQueue || _selection==currentQueue+1){
					_saveOrLoadSlot--;
					sprintf(_slotString," - slot %d]",_saveOrLoadSlot);
					_compiledOptionsPath = getOptionsFileLocation(_saveOrLoadSlot,_specificOptionsNumber);
					_currentSlotExists = checkFileExist(_compiledOptionsPath);
				}
			}
		}else if (WasJustPressed(SCE_CTRL_RIGHT)){
			if (_selection<currentQueue){
				if (inputTypeQueue[_selection]==INPUTTYPENUMBER){
					(*((int*)(userInputResults[_selection])))++;
					pushUserInput(passedState,userInputResults[_selection],inputTypeQueue[_selection],_selection+1);
				}
			}else{
				if (_selection==currentQueue || _selection==currentQueue+1){
					_saveOrLoadSlot++;
					sprintf(_slotString," - slot %d]",_saveOrLoadSlot);
					_compiledOptionsPath = getOptionsFileLocation(_saveOrLoadSlot,_specificOptionsNumber);
					_currentSlotExists = checkFileExist(_compiledOptionsPath);
				}
			}
		}else if (WasJustPressed(SCE_CTRL_CROSS)){
			if (_saveAndLoadEnabled==1){
				if (_selection==currentQueue){
					_currentSlotExists=1;
					FILE* fp = fopen(_compiledOptionsPath,"w");
					if (lua_getglobal(passedState,"SCRIPTVERSION")==LUA_TNUMBER){
						fprintf(fp,"%d\n",(int)lua_tonumber(passedState,1));
					}else{
						printf("!!!!!!!!!!!!!!!\nSCRIPTVERSION\nnot found. PLZ FIX!\n!!!!!!!!!!");
						fprintf(fp,"%d\n",0);
					}
					lua_pop(passedState,1);
					if (lua_getglobal(passedState,"SAVEVARIABLE")==LUA_TNUMBER){
						fprintf(fp,"%d\n",(int)lua_tonumber(passedState,-1));
					}else{
						printf("!!!!!!!!!!!!!!!\nSAVEVARIABLE\nnot found. PLZ FIX!\n!!!!!!!!!!");
						fprintf(fp,"%d\n",0);
					}
					lua_pop(passedState,1);
					for (i=0;i<currentQueue;i++){
						if (inputTypeQueue[i]==INPUTTYPENUMBER){
							fprintf(fp,"%d\n",*((int*)userInputResults[i]));
						}else if (inputTypeQueue[i]==INPUTTYPELIST){
							if (_listEntries[i]==NULL){
								fprintf(fp,"%s\n","(Undefined.)");
							}else{
								fprintf(fp,"%s\n",_listEntries[i][(*((int*)userInputResults[i]))-1]);
							}
						}else if (inputTypeQueue[i]==INPUTTYPESTRING){
							if (userInputResults[i]==NULL){
								fprintf(fp,"%s\n","(empty)");
							}else{
								fprintf(fp,"%s\n",(char*)userInputResults[i]);
							}
						}else{ // INPUTTYPELISTMULTI
							fprintf(fp,"%s\n","(non-saveable type)");
						}
					}
					fclose(fp); // Save button
					popupMessage("Saved.",0,0);
				}else if (_selection==currentQueue+1){
					if (_currentSlotExists==1){
						FILE* fp = fopen(_compiledOptionsPath,"r");
						char _lastReadLine[256];
						fgets(_lastReadLine, sizeof(_lastReadLine), fp);
						lua_pushstring(passedState,_lastReadLine);
						lua_setglobal(passedState, "loadedScriptVersion");
						
						fgets(_lastReadLine, sizeof(_lastReadLine), fp);
						lua_pushstring(passedState,_lastReadLine);
						lua_setglobal(passedState, "loadedSaveVariable");
						i=1;
						while (fgets(_lastReadLine, sizeof(_lastReadLine), fp)) {
							removeNewline(_lastReadLine);
							// Push
							char _userInputResultName[256];
							sprintf(_userInputResultName,"userLoad%02d",i);
							lua_pushstring(passedState,_lastReadLine);
							lua_setglobal(passedState, _userInputResultName);
							i++;
						}
						fclose(fp);
						// Pushes number of prompts so far
						lua_pushnumber(passedState,numberOfPrompts);
						lua_setglobal(passedState,"numberOfPrompts");
						// Adds function to stack
						if (lua_getglobal(passedState,"onOptionsLoad")!=0){
							lua_call(passedState, 0, 0);
						}else{
							lua_pop(passedState,1);
						}
						// Call all list init functions
						for (i=0;i<currentQueue;i++){
							if (inputTypeQueue[i]==INPUTTYPELIST || inputTypeQueue[i]==INPUTTYPELISTMULTI){
								callListInit(passedState,i+1,2);
								_listEntriesLength[i] = assignAfterListInit(passedState,&(_listEntries[i]),_listEntriesLength[i],-1);
							}
						}
						popupMessage("Loaded.",0,0);
					}else{
						popupMessage("Slot file does not exist.",1,0);
					} // Load button
				}else if (_selection>=currentQueue+2){
					break;
				}
			}else{
				if (_selection>=currentQueue){
					break;
				}
			}
			if (_selection<currentQueue){
				if (inputTypeQueue[_selection]==INPUTTYPELIST || inputTypeQueue[_selection]==INPUTTYPELISTMULTI){
					// This is only the list initialization. Look past the "else if" statements to find another if statement that contains the code for showing the list and stuff.
					int _currentList=_selection;
					callListInit(passedState,_currentList+1,_listEntries[_currentList]==NULL);
					_listEntriesLength[_currentList] = assignAfterListInit(passedState,&(_listEntries[_currentList]),_listEntriesLength[_currentList],-1);
					// Code specific to list or multi list.
					if (_listEntries[_currentList]!=NULL){
						if (inputTypeQueue[_currentList]==INPUTTYPELIST){
							int _tempUserAnswer = showList(_listEntries[_currentList],_listEntriesLength[_currentList],*((int*)userInputResults[_currentList])-1,NULL); // Subtract 1 from starting index because result was one based.
							if (_tempUserAnswer!=-1){
								*((int*)(userInputResults[_currentList]))=_tempUserAnswer;
								//int* _noob = malloc(4);
								//*_noob = 1;
								pushUserInput(passedState,userInputResults[_currentList],inputTypeQueue[_currentList],_currentList+1);
							}
						}else if (inputTypeQueue[_currentList]==INPUTTYPELISTMULTI){
							NathanLinkedList* _tempUserAnswer = (NathanLinkedList*)showList(_listEntries[_currentList],_listEntriesLength[_currentList],((int*)(((NathanLinkedList*)userInputResults[_currentList])->memory))!=NULL ? *((int*)(((NathanLinkedList*)userInputResults[_currentList])->memory))-1 : 0,(NathanLinkedList*)userInputResults[_currentList]);
							if ((signed int)_tempUserAnswer!=-1){
								userInputResults[_currentList] = _tempUserAnswer;
								if (((NathanLinkedList*)userInputResults[_currentList])->memory!=NULL){
									int _cachedListLength=getLinkedListLength((NathanLinkedList*)userInputResults[_currentList]);
									lua_createtable(passedState, _cachedListLength, 0);
									for (i=0;i<_cachedListLength;i++){
										NathanLinkedList* _lastGottenList = getLinkedList((NathanLinkedList*)userInputResults[_currentList],i+1);
										lua_pushnumber(L,i+1);
										lua_pushnumber(L, *((int*)_lastGottenList->memory));
										lua_settable(L, -3); // stack(-3)[stack(-2)] = stack(-1); // someTable[1]="first elemenbt string"
									}
								}else{
									// We'll push an empty table.
									lua_createtable(passedState, 0, 0);
								}
								pushUserInput(passedState,NULL,INPUTTYPELISTMULTI,_currentList+1);
							}
						}
						callListFinish(passedState,_currentList+1);
					}
				}else if (inputTypeQueue[_selection]==INPUTTYPENUMBER){
					*((int*)(userInputResults[_selection])) = inputNumber(*((int*)(userInputResults[_selection])));
					pushUserInput(passedState,userInputResults[_selection],inputTypeQueue[_selection],_selection+1);
				}else if (inputTypeQueue[_selection]==INPUTTYPESTRING){
					userInputResults[_selection] = userKeyboardInput(userInputResults[_selection]!=NULL ? userInputResults[_selection] : "",shortNameQueue[_selection],99);
					if (userInputResults[_selection]!=NULL){
						pushUserInput(passedState,userInputResults[_selection],inputTypeQueue[_selection],_selection+1);
					}
				}
			}
		}else if (WasJustPressed(SCE_CTRL_CIRCLE)){
			_userDidQuit=1;
			break;
		}else if (WasJustPressed(SCE_CTRL_SQUARE)){
			if (_selection<currentQueue){
				popupMessage(longNameQueue[_selection],1,0);
			}
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
					GoodDrawTextColored(_currentDrawWidth,currentTextHeight*i,_listEntries[i][*((int*)userInputResults[i])-1],fontSize,COLORSELECTED);
				}
			}else if (inputTypeQueue[i]==INPUTTYPENUMBER){
				char _tempNumberBuffer[10];
				gooditoa(*((int*)userInputResults[i]),_tempNumberBuffer,10);
				GoodDrawTextColored(_currentDrawWidth,currentTextHeight*i,_tempNumberBuffer,fontSize,COLORSELECTED);
			}else if (inputTypeQueue[i]==INPUTTYPESTRING){
				if (userInputResults[i]!=NULL){
					GoodDrawTextColored(_currentDrawWidth,currentTextHeight*i,userInputResults[i],fontSize,COLORSELECTED);
				}
			}
		}
		if (_saveAndLoadEnabled==1){
			GoodDrawTextColored(cursorWidth+5,currentTextHeight*i,"[Save",fontSize,COLOROPTION);
			GoodDrawTextColored(cursorWidth+5,currentTextHeight*(i+1),"[Load",fontSize,COLOROPTION);
			if (_currentSlotExists==1){
			GoodDrawTextColored(cursorWidth+5+TextWidth(fontSize,"[Save"),currentTextHeight*i,_slotString,fontSize,COLORMAYBE);
			GoodDrawTextColored(cursorWidth+5+TextWidth(fontSize,"[Load"),currentTextHeight*(i+1),_slotString,fontSize,COLORVALID);
			}else{
			GoodDrawTextColored(cursorWidth+5+TextWidth(fontSize,"[Save"),currentTextHeight*i,_slotString,fontSize,COLORVALID);
			GoodDrawTextColored(cursorWidth+5+TextWidth(fontSize,"[Load"),currentTextHeight*(i+1),_slotString,fontSize,COLORINVALID);
			}
			GoodDrawTextColored(cursorWidth+5,currentTextHeight*(i+2),"Done",fontSize,COLOROPTION);
		}else{

			GoodDrawTextColored(cursorWidth+5,currentTextHeight*i,"Done",fontSize,COLOROPTION);
		}
		EndDrawing();
		
		FpsCapWait();
	}
	// Free memory
	free(_compiledOptionsPath);
	freeQueue(currentQueue);
	for (i=0;i<currentQueue;i++){
		if (userInputResults[currentQueue]!=NULL){ // Empty string inputs will be NULL pointer
			if (inputTypeQueue[currentQueue]==INPUTTYPELISTMULTI){
				if (((NathanLinkedList*)userInputResults[i])->memory!=NULL){
					freeLinkedList(userInputResults[i]);
				}
			}else{
				free(userInputResults[i]);
			}
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
	popupMessage(lua_tostring(passedState,1),1,0);
	return 0;
}
int L_showStatus(lua_State* passedState){
	popupMessage(lua_tostring(passedState,-1),0,0);
	return 0;
}
int L_writeToDebugFile(lua_State* passedState){
	WriteToDebugFile(lua_tostring(passedState,1));
	return 0;
}
int L_disableSSL(lua_State* passedState){
	disableSSL();
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
	LUAREGISTER(L_setUserInput,"setUserInput");
	LUAREGISTER(L_printListStuff,"printListStuff");
	LUAREGISTER(L_assignListData,"assignListData");
	LUAREGISTER(L_writeToDebugFile,"WriteToDebugFile");
	LUAREGISTER(L_disableSSL,"disableSSL");
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

	// Construct fixed paths
	FixPath(CONSTANTDOWNLOADERSLOCATION,tempPathFixBuffer,TYPE_EMBEDDED);
	downloadersLocation = malloc(strlen(tempPathFixBuffer)+1);
	strcpy(downloadersLocation,tempPathFixBuffer);
	FixPath(CONSTANTMANGAFOLDERROOT,tempPathFixBuffer,TYPE_DATA);
	mangaFolderRoot = malloc(strlen(tempPathFixBuffer)+1);
	strcpy(mangaFolderRoot,tempPathFixBuffer);
	FixPath(CONSTANTOPTIONSFOLDERROOT,tempPathFixBuffer,TYPE_DATA);
	optionsLocation = malloc(strlen(tempPathFixBuffer)+1);
	strcpy(optionsLocation,tempPathFixBuffer);

	createDirectory(mangaFolderRoot);
	createDirectory(optionsLocation);

	FixPath("assets/Init.lua",tempPathFixBuffer,TYPE_EMBEDDED);
	luaL_dofile(L,tempPathFixBuffer);
	FixPath("assets/GlobalTracking.lua",tempPathFixBuffer,TYPE_EMBEDDED);
	luaL_dofile(L,tempPathFixBuffer);
}
int main(int argc, char *argv[]){
	init();
	char* _userChosenScriptFile = chooseScript();
	if (_userChosenScriptFile==NULL){
		return 0;
	}
	// Start
	doScript(_userChosenScriptFile);
	free(_userChosenScriptFile);
	// End
	quitApplication();
	return 0;
}
