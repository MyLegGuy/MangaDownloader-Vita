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

#include "PlatformConfig.h"

#if PLATFORM == PLAT_VITA
	#include <psp2/ctrl.h>
	#include <psp2/kernel/processmgr.h>
	#include <psp2/rtc.h>
	#include <psp2/types.h>
	#include <psp2/touch.h>
	#include <psp2/io/fcntl.h>
	#include <psp2/io/dirent.h>
	#include <psp2/power.h>
#endif

#include "CrossPlatformGeneral.h"
#include "Download.h"

/*============================================================================*/
lua_State* L;
char* shortNameQueue[5];
char* longNameQueue[5];
char inputTypeQueue[5];
int currentQueue=0;

//===============
// OPTIONS
//===============
char downloadCoverIfPossible=1;

/*============================================================================*/
void quitApplication(){
	curl_easy_cleanup(curl_handle);
}
// Removes all 0x0D and 0x0A from last two characters of string by moving null character.
void removeNewline(char** _toRemove){
	int _cachedStrlen = strlen(*_toRemove);
	int i;
	for (i=0;i!=2;i++){
		if (!(((*_toRemove)[_cachedStrlen-(i+1)]==0x0A) || ((*_toRemove)[_cachedStrlen-(i+1)]==0x0D))){
			break;
		}
	}
	(*_toRemove)[_cachedStrlen-i] = '\0';
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
	MakeDirectory(lua_tostring(passedState,1));
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
int L_waitForUserInputs(lua_State* passedState){
	int i;
	void* userInputResults[currentQueue]; // Memory is dynamiclly allocated
	char lastTypedLine[256];
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
	char _currentQueueBackup = currentQueue;
	currentQueue=0;
	return _currentQueueBackup;
}
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
}
int main(int argc, char *argv[]){
	init();
	MakeDirectory(MANGAFOLDERROOT);
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
	quitApplication();
	return 0;
}
/*
TOOD - Cover downloading
*/