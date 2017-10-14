#ifndef PHOTOEXTENDEDHEADER
#define PHOTOEXTENDEDHEADER
///////////////////////////

#include <Lua/lua.h>
#include <Lua/lualib.h>
#include <Lua/lauxlib.h>

volatile signed char needUpdateFileListing=-1;
volatile unsigned short totalDownloadedFiles=0;
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
// Doesn't actually get extention. Just returns last few characters of string.
char* getFileExtention(char* _filename, int _extentionLength){
	if (strlen(_filename)<_extentionLength){
		return _filename;
	}
	return &(_filename[strlen(_filename)-_extentionLength]);
}
// Called by image viewing thread
int loadNewPage(CrossTexture** _toStorePage, char** _currentRelativeFilename, int _currentOffset){
	int i, j;
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
		// Alphabetize
		for (i = 0; i < _mangaDirectoryLength-1 ; i++){ // minus one because no need to check last file
			for (j = i; j < _mangaDirectoryLength; j++){
				if (strcmp(_mangaDirectoryFilenames[i], _mangaDirectoryFilenames[j]) > 0){ // Move up next one if less than this one
					char* _tempBuffer = malloc(strlen(_mangaDirectoryFilenames[i]+1));
					strcpy(_tempBuffer, _mangaDirectoryFilenames[i]);
					free(_mangaDirectoryFilenames[i]);
					_mangaDirectoryFilenames[i] = malloc(strlen(_mangaDirectoryFilenames[j])+1);
					strcpy(_mangaDirectoryFilenames[i],_mangaDirectoryFilenames[j]);
					free(_mangaDirectoryFilenames[j]);
					_mangaDirectoryFilenames[j] = _tempBuffer;
				}
			}
		}
	}

	if (totalDownloadedFiles>_mangaDirectoryLength){
		popupMessage("This...is so Rong.\nBy that, I mean there are less files than there should be.",1,0);
		return LOADNEW_RETURNEDSAME;
	}
	_mangaDirectoryLength=totalDownloadedFiles;

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
			if (*_toStorePage!=NULL){
				FreeTexture(*_toStorePage);
				*_toStorePage=NULL;
			}
			if (*_currentRelativeFilename!=NULL){
				free(*_currentRelativeFilename);
				*_currentRelativeFilename=NULL;
			}
			return LOADNEW_FINISHEDMANGA;
		}
		popupMessage("Waiting for the next page.\nYou may go to the previous page if you wish.",0,0);
		unsigned short _cacheTotalDownloadedFiles = totalDownloadedFiles;
		ControlsStart();
		ControlsEnd();
		while (_cacheTotalDownloadedFiles==totalDownloadedFiles){
			sceKernelDelayThread(350000); // Wait one fourth of a second
			ControlsStart();
			if (WasJustPressed(SCE_CTRL_UP) || WasJustPressed(SCE_CTRL_LEFT)){
				// from vitashell
				readPad();
				readPad();
				return LOADNEW_RETURNEDSAME;
			}
			ControlsEnd();
		}
		return loadNewPage(_toStorePage,_currentRelativeFilename,_currentOffset);
	}

	char* _tempPathFixBuffer = malloc(strlen(_mangaDirectoryFilenames[_startIndex])+strlen(currentDownloadReaderDirectory)+1);
	strcpy(_tempPathFixBuffer,currentDownloadReaderDirectory);
	strcat(_tempPathFixBuffer,_mangaDirectoryFilenames[_startIndex]);
	WriteToDebugFile(_tempPathFixBuffer);
	if (strcmp(getFileExtention(_tempPathFixBuffer,3),"jpg")==0){
		if (*_toStorePage!=NULL){
			FreeTexture(*_toStorePage);
			*_toStorePage=NULL;
		}
		*_toStorePage = LoadJPG(_tempPathFixBuffer);
	}else if (strcmp(getFileExtention(_tempPathFixBuffer,3),"png")==0){
		if (*_toStorePage!=NULL){
			FreeTexture(*_toStorePage);
			*_toStorePage=NULL;
		}
		*_toStorePage = LoadPNG(_tempPathFixBuffer);
	}else{
		popupMessage("Unknwon filetype.",1,0);
		popupMessage(getFileExtention(_tempPathFixBuffer,3),1,0);
		return LOADNEW_RETURNEDSAME;
	}
	free(_tempPathFixBuffer);
	if (*_currentRelativeFilename!=NULL){
		free(*_currentRelativeFilename);
	}
	*_currentRelativeFilename = malloc(strlen(_mangaDirectoryFilenames[_startIndex])+1);
	strcpy(*_currentRelativeFilename,_mangaDirectoryFilenames[_startIndex]);
	//return _mangaDirectoryFilenames[_userChosenFileIndex-1];
	


	return LOADNEW_LOADEDNEW;
}
//////
#endif