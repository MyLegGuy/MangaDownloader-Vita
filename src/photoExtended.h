#ifndef PHOTOEXTENDEDHEADER
#define PHOTOEXTENDEDHEADER
///////////////////////////

#include <Lua/lua.h>
#include <Lua/lualib.h>
#include <Lua/lauxlib.h>

volatile signed char needUpdateFileListing=-1;
volatile signed short totalDownloadedFiles=0;
volatile unsigned char isDoneDownloading=0;
char* currentDownloadReaderDirectory=NULL;

char* _mangaDirectoryFilenames[MAXFILES] = {NULL};
int _mangaDirectoryLength=0;

int L_requireNewDirectorySearch(lua_State* passedState){
	needUpdateFileListing=1;
	return 0;
}
int L_incrementTotalDownloadedFiles(lua_State* passedState){
	totalDownloadedFiles+=lua_tonumber(passedState,1);
	return 0;
}
int L_setMangaDoneDownloading(lua_State* passedState){
	isDoneDownloading = lua_toboolean(passedState,1);
	return 0;
}

#define LOADNEW_RETURNEDSAME 0
#define LOADNEW_LOADEDNEW 1
#define LOADNEW_DIDNTLOAD 2
#define LOADNEW_FINISHEDMANGA 3
// Called by image viewing thread
int loadNewPage(CrossTexture** _toStorePage, char** _currentRelativeFilename, int _currentOffset){
	int i;
	if (needUpdateFileListing){
		_mangaDirectoryLength=0;
		needUpdateFileListing=0;
		// Select download script
		CROSSDIR dir;
		CROSSDIRSTORAGE lastStorage;
		dir = openDirectory (currentDownloadReaderDirectory);
		if (dirOpenWorked(dir)==0){
			popupMessage("Directory missing.",1,0);
			return LOADNEW_DIDNTLOAD;
		}
		for (i=0;i<MAXFILES;i++){
			if (directoryRead(&dir,&lastStorage) == 0){
				break;
			}
			_mangaDirectoryFilenames[i] = malloc(strlen(getDirectoryResultName(&lastStorage))+1);
			strcpy(_mangaDirectoryFilenames[i],getDirectoryResultName(&lastStorage));
			_mangaDirectoryLength++;
		}
		directoryClose (dir);
		if (_mangaDirectoryLength==0){
			return LOADNEW_DIDNTLOAD;
		}
		alphabetizeList(_mangaDirectoryFilenames,_mangaDirectoryLength);
	}
	if (totalDownloadedFiles!=-1){
		if (totalDownloadedFiles>_mangaDirectoryLength){
			popupMessage("This...is so Rong. By that, I mean there are less files than there should be.",1,0);
			return LOADNEW_RETURNEDSAME;
		}
		_mangaDirectoryLength=totalDownloadedFiles;
	}
	int _startIndex=0;
	if (*_currentRelativeFilename!=NULL){
		for (i=0;i<_mangaDirectoryLength;i++){
			if (strcmp(_mangaDirectoryFilenames[i],*_currentRelativeFilename)==0){
				_startIndex=i;
				break;
			}
		}
	}
	_startIndex+=_currentOffset;
	if (_startIndex<0){
		return LOADNEW_RETURNEDSAME;
	}
	if (_startIndex>=_mangaDirectoryLength){
		if (isDoneDownloading==1){
			return LOADNEW_FINISHEDMANGA;
		}
		popupMessage("Waiting for the next page.\nYou may go to the previous page if you wish.",0,0);
		unsigned short _cacheTotalDownloadedFiles = totalDownloadedFiles;
		controlsStart();
		controlsEnd();
		while (_cacheTotalDownloadedFiles==totalDownloadedFiles){
			sceKernelDelayThread(350000); // Wait one fourth of a second
			controlsStart();
			if (wasJustPressed(SCE_CTRL_UP) || wasJustPressed(SCE_CTRL_LEFT)){
				// from vitashell
				readPad();
				readPad();
				return LOADNEW_RETURNEDSAME;
			}
			controlsEnd();
		}
		return loadNewPage(_toStorePage,_currentRelativeFilename,_currentOffset);
	}
	if (*_toStorePage!=NULL){
		freeTexture(*_toStorePage);
		*_toStorePage=NULL;
	}
	char* _tempPathFixBuffer = malloc(strlen(_mangaDirectoryFilenames[_startIndex])+strlen(currentDownloadReaderDirectory)+1);
	strcpy(_tempPathFixBuffer,currentDownloadReaderDirectory);
	strcat(_tempPathFixBuffer,_mangaDirectoryFilenames[_startIndex]);
	if (strcmp(getFileExtention(_tempPathFixBuffer,3),"jpg")==0){
		*_toStorePage = loadJPG(_tempPathFixBuffer);
	}else if (strcmp(getFileExtention(_tempPathFixBuffer,3),"png")==0){
		*_toStorePage = loadPNG(_tempPathFixBuffer);
	}else{
		return loadNewPage(_toStorePage,_currentRelativeFilename,_currentOffset+1);
	}
	free(_tempPathFixBuffer);
	if (*_currentRelativeFilename!=NULL){
		free(*_currentRelativeFilename);
	}
	*_currentRelativeFilename = malloc(strlen(_mangaDirectoryFilenames[_startIndex])+1);
	strcpy(*_currentRelativeFilename,_mangaDirectoryFilenames[_startIndex]);
	return LOADNEW_LOADEDNEW;
}
//////
#endif