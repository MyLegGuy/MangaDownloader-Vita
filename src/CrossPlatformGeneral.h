#ifndef CROSSPLATFORMGENERALH
#define CROSSPLATFORMGENERALH

	#if PLATFORM == PLAT_WINDOWS
		#include <dirent.h>
	#endif

	signed char checkFileExist(const char* location){
		#if PLATFORM == PLAT_VITA
			SceUID fileHandle = sceIoOpen(location, SCE_O_RDONLY, 0777);
			if (fileHandle < 0){
				return 0;
			}else{
				sceIoClose(fileHandle);
				return 1;
			}
		#elif PLATFORM == PLAT_WINDOWS
			if( access( location, F_OK ) != -1 ) {
				return 1;
			} else {
			    return 0;
			}
		#endif
	}

	void MakeDirectory(const char* path){
		#if PLATFORM == PLAT_VITA
			sceIoMkdir(path,0777);
		#elif PLATFORM == PLAT_WINDOWS
			#if SUBPLATFORM == SUB_ANDROID
				mkdir(path,0777);
			#else
				mkdir(path);
			#endif
		#endif
	}

	#if PLATFORM == PLAT_WINDOWS
		#define CROSSDIR DIR*
		#define CROSSDIRSTORAGE struct dirent*
	#elif PLATFORM == PLAT_VITA
		#define CROSSDIR SceUID
		#define CROSSDIRSTORAGE SceIoDirent
	#endif

	char DirOpenWorked(CROSSDIR passedir){
		#if PLATFORM == PLAT_WINDOWS
			if (passedir==NULL){
				return 0;
			}
		#elif PLATFORM == PLAT_VITA
			if (passedir<0){
				return 0;
			}
		#endif
		return 1;
	}

	CROSSDIR OpenDirectory(const char* filepath){
		#if PLATFORM == PLAT_WINDOWS
			return opendir(filepath);
		#elif PLATFORM == PLAT_VITA
			return (sceIoDopen(filepath));
		#endif
	}

	char* GetDirectoryResultName(CROSSDIRSTORAGE* passedStorage){
		#if PLATFORM == PLAT_WINDOWS
			return ((*passedStorage)->d_name);
		#elif PLATFORM == PLAT_VITA
			//WriteToDebugFile
			return ((passedStorage)->d_name);
		#endif
	}

	int DirectoryRead(CROSSDIR* passedir, CROSSDIRSTORAGE* passedStorage){
		#if PLATFORM == PLAT_WINDOWS
			*passedStorage = readdir (*passedir);
			if (*passedStorage != NULL){
				if (strcmp((*passedStorage)->d_name,".")==0 || strcmp((*passedStorage)->d_name,"..")==0){
					return DirectoryRead(passedir,passedStorage);
				}
			}
			if (*passedStorage == NULL){
				return 0;
			}else{
				return 1;
			}
		#elif PLATFORM == PLAT_VITA
			int _a = sceIoDread(*passedir,passedStorage);
			return _a;
			
		#endif
	}

	void DirectoryClose(CROSSDIR passedir){
		#if PLATFORM == PLAT_WINDOWS
			closedir(passedir);
		#elif PLATFORM == PLAT_VITA
			sceIoDclose(passedir);
		#endif
	}

	char DirectoryExists(const char* filepath){
		CROSSDIR _tempdir = OpenDirectory(filepath);
		if (DirOpenWorked(_tempdir)==1){
			DirectoryClose(_tempdir);
			return 1;
		}else{
			return 0;
		}
	}

#endif