#define VERSION 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
#include "Downloader.h"

/////////////////////////////////////////////////////////
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
/*
Loads image at start.
Frees at end.
Returns the next action to do. (Exit, go to next page, go to previous page, etc)
SETTINGS ARE IN GLOBAL VARIABLE
*/
int readPage(char* _filepath){
	return 3;
}

/////////////////////////////////////////////////////////
void init(){
	ClearDebugFile();
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
}
int main(int argc, char *argv[]){
	init();


	initDownloadBroad();
	doScript(chooseScript());
	return 0;
}
