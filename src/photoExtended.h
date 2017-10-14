#ifndef PHOTOEXTENDEDHEADER
#define PHOTOEXTENDEDHEADER
///////////////////////////
volatile signed char needUpdateFileListing=-1;
char* currentDownloadReaderDirectory=NULL;

char* _mangaDirectoryFilenames[MAXFILES] = {NULL};
int _mangaDirectoryLength=0;

#define LOADNEW_RETURNEDSAME 0
#define LOADNEW_LOADEDNEW 1
#define LOADNEW_DIDNTLOAD 2
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
		// TODO - Elegantly tell the user that the next file is not ready.
			// TODO - Also must be able to tell if it's the end of the manga.
		popupMessage("Not ready.",0,0);
		// Just to be safe.
		//needUpdateFileListing=1;
		return LOADNEW_RETURNEDSAME;
	}
	char* _tempPathFixBuffer = malloc(strlen(_mangaDirectoryFilenames[_startIndex])+strlen(currentDownloadReaderDirectory)+1);
	strcpy(_tempPathFixBuffer,currentDownloadReaderDirectory);
	strcat(_tempPathFixBuffer,_mangaDirectoryFilenames[_startIndex]);
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