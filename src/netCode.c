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

#include <stdlib.h>
#include <string.h>
#include <goodbrew/config.h>

#if GBPLAT == GB_VITA
	#include <psp2/sysmodule.h>
	#include <psp2/kernel/processmgr.h>
	#include <psp2/display.h>
	
	#include <psp2/net/net.h>
	#include <psp2/net/netctl.h>
	#include <psp2/net/http.h>
	
	#include <psp2/io/fcntl.h>
#endif

#if DOWNLOADTYPE == DOWNLOAD_CURL
	#include "main.h"

	#include <curl/curl.h>
	typedef struct grhuigruei{
		char *memory;
		size_t size;
	}MemoryStruct;
	CURL* curl_handle;
	char curlFollowRedirects=0;

	void cleanupNetCode(){
		curl_easy_cleanup(curl_handle);
	}

	size_t curlWriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp){
		size_t realsize = size * nmemb;
		MemoryStruct* mem = (MemoryStruct*) userp;

		char* _newBuffer = realloc(mem->memory, mem->size + realsize + 1);
		if(_newBuffer == NULL) {
			free(mem->memory);
			/* out of memory! */
			WriteToDebugFile("not enough memory (realloc returned NULL)\n");
			return 0;
		}
		mem->memory = _newBuffer;
	
		memcpy(&(mem->memory[mem->size]), contents, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
	
		return realsize;
	}
	size_t curlWriteDataFile(void *ptr, size_t size, size_t nmemb, void *stream){
		#if GBPLAT == GB_VITA
			sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
		#endif
		size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
		return written;
	}
	void downloadToFile(const char* passedUrl, const char* passedFilename){
		FILE* fp;
		curl_easy_setopt(curl_handle, CURLOPT_URL, passedUrl);
		fp = fopen(passedFilename, "wb");
		if(fp) {
			curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curlWriteDataFile);
			curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fp);
			curl_easy_perform(curl_handle);
			fclose(fp);
		}
	}
	void downloadWebpageData(const char* url, char** _toStoreWebpageData, size_t* _toStoreSize){
		CURLcode res;
		MemoryStruct chunkToDownloadTo;
		chunkToDownloadTo.memory = malloc(1);  /* will be grown as needed by the realloc above */
		chunkToDownloadTo.size = 0;    /* no data at this point */
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curlWriteMemoryCallback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunkToDownloadTo);
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, curlFollowRedirects);
		res = curl_easy_perform(curl_handle);
		if(res != CURLE_OK) {
			WriteToDebugFile("Failed, the world is over.\n");
			WriteIntToDebugFile((int)res);
		}
		*_toStoreWebpageData = chunkToDownloadTo.memory;
		if (_toStoreSize!=NULL){
			*_toStoreSize = chunkToDownloadTo.size;
		}
	}
	void disableSSLVerification(){
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
	}
	void setReferer(const char* _newReferer){
		curl_easy_setopt(curl_handle, CURLOPT_REFERER, _newReferer);
	}
	void setUserAgent(const char* _agentName){
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, _agentName);
	}
	void downloadEnableDebugInfo(){
		curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
	}
	void downloadSetRedirects(char _isEnabled){
		curlFollowRedirects = _isEnabled;
	}
	char* downloadGetLastRedirect(){
		if (curlFollowRedirects==0){
			char* _returnLocation;
			CURLcode res;
			res = curl_easy_getinfo(curl_handle, CURLINFO_REDIRECT_URL, &_returnLocation);
			if((res == CURLE_OK) && _returnLocation) {
				return _returnLocation;
			}else{
				printf("Erorr in get redirect url");
				return NULL;
			}
		}else{
			printf("Can't work, you're already following redirects");
			return NULL;
		}
	}
	void initDownload(char* _certLocation){
		#if GBPLAT == GB_VITA
			sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
			SceNetInitParam netInitParam;
			int size = 4*1024*1024;
			netInitParam.memory = malloc(size);
			netInitParam.size = size;
			netInitParam.flags = 0;
			sceNetInit(&netInitParam);
			sceNetCtlInit();
			sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
			sceHttpInit(4*1024*1024);
		#endif
		curl_global_init(CURL_GLOBAL_ALL);
		curl_handle = curl_easy_init();
		//curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
		//curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
		
		// Needed for https
		if (_certLocation!=NULL){
			curl_easy_setopt(curl_handle, CURLOPT_CAINFO, _certLocation);
		}else{
			curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
		}
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	}
#elif DOWNLOADTYPE == DOWNLOAD_VITA
	
#endif
