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
int loadNewPageFile(CrossTexture** _toStorePage, char** _currentRelativeFilename, int _currentOffset){
	popupMessage("Loading...",0,0);
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
			popupMessage("This...is so Rong. By that, I mean there are fewer files than there should be.",1,0);
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
		return loadNewPageFile(_toStorePage,_currentRelativeFilename,_currentOffset);
	}
	if (*_toStorePage!=NULL){
		freeTexture(*_toStorePage);
		*_toStorePage=NULL;
	}
	char* _tempPathFixBuffer = malloc(strlen(_mangaDirectoryFilenames[_startIndex])+strlen(currentDownloadReaderDirectory)+1);
	strcpy(_tempPathFixBuffer,currentDownloadReaderDirectory);
	strcat(_tempPathFixBuffer,_mangaDirectoryFilenames[_startIndex]);
	if (hasImageExtension(_tempPathFixBuffer)){
		*_toStorePage = loadLoadableImage(_tempPathFixBuffer);
	}else{
		return loadNewPageFile(_toStorePage,_currentRelativeFilename,_currentOffset+1);
	}
	free(_tempPathFixBuffer);
	if (*_currentRelativeFilename!=NULL){
		free(*_currentRelativeFilename);
	}
	*_currentRelativeFilename = malloc(strlen(_mangaDirectoryFilenames[_startIndex])+1);
	strcpy(*_currentRelativeFilename,_mangaDirectoryFilenames[_startIndex]);
	return LOADNEW_LOADEDNEW;
}

#if RENDERER == REND_VITA2D
#include <jpeglib.h>
#include <png.h>
#define PNG_SIGSIZE (8)
vita2d_texture *_vita2d_load_JPEG_generic(struct jpeg_decompress_struct *jinfo, struct jpeg_error_mgr *jerr);
vita2d_texture *_vita2d_load_PNG_generic(const void *io_ptr, png_rw_ptr read_data_fn);

void _png_read_callback(png_structp png_ptr, png_bytep data, png_size_t length)
{
	struct decstate* d = png_get_io_ptr(png_ptr);
	decryptmore(d,data,length);
}
#endif

int loadNewPageArchive(CrossTexture** _toStorePage, char** _currentRelativeFilename, int _directionOffset, struct decstate* d){
	uint64_t _len = dread64(d);
	if (*_toStorePage!=NULL){
		freeTexture(*_toStorePage);
		*_toStorePage=NULL;
	}
	if (decryptioneof(d)){
		return LOADNEW_FINISHEDMANGA;
	}
	
	#if RENDERER == REND_SDL
	unsigned char* _bytes = malloc(_len);
	decryptmore(d,_bytes,_len);
	if (_bytes[0]==0x89){ // png
		*_toStorePage=loadPNGBuffer(_bytes,_len);
	}else if (_bytes[0]==0xFF){ // jpg
		*_toStorePage=loadJPGBuffer(_bytes,_len);
	}
	free(_bytes);
	#elif RENDERER == REND_VITA2D

	unsigned char _firstByte;
	decryptmore(d,&_firstByte,1);
	if (_firstByte==0x89){ // png
		png_byte pngsig[PNG_SIGSIZE];
		if (png_sig_cmp(pngsig,0,PNG_SIGSIZE)!=0){
			fprintf(stderr,"bad png\n");
		}
		*_toStorePage=_vita2d_load_PNG_generic(d, _png_read_callback);
	}else if (_firstByte==0xFF){
	}else{
		// TODO - either just fast forward through the rest of the file OR make sure only png or jpg end up in archive.
	}
	
	#else
	#warning no loader for this
	#endif
	return LOADNEW_RETURNEDSAME;
}

// Called by image viewing thread
int loadNewPage(CrossTexture** _toStorePage, char** _currentRelativeFilename, int _directionOffset, struct decstate* d){
	if (d){
		if (_directionOffset<0){
			return LOADNEW_RETURNEDSAME;
		}
		return loadNewPageArchive(_toStorePage,_currentRelativeFilename,_directionOffset,d);
	}else{
		return loadNewPageFile(_toStorePage,_currentRelativeFilename,_directionOffset);
	}
	
}

//////
#endif
