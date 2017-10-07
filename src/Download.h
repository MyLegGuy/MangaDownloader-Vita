#ifndef DOWNLOADHEADER
#define DOWNLOADHEADER

	#if PLATFORM == PLAT_VITA
		#include <psp2/sysmodule.h>
		#include <psp2/kernel/processmgr.h>
		#include <psp2/display.h>
		
		#include <psp2/net/net.h>
		#include <psp2/net/netctl.h>
		#include <psp2/net/http.h>
		
		#include <psp2/io/fcntl.h>
	#endif

	#if DOWNLOADTYPE == DOWNLOAD_CURL
		#include <curl/curl.h>
		typedef struct grhuigruei{
			char *memory;
			size_t size;
		}MemoryStruct;
		CURL* curl_handle;

		size_t curlWriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp){
			size_t realsize = size * nmemb;
			MemoryStruct* mem = (MemoryStruct*) userp;
			mem->memory = realloc(mem->memory, mem->size + realsize + 1);
			if(mem->memory == NULL) {
				free(mem->memory);
				/* out of memory! */
				WriteToDebugFile("not enough memory (realloc returned NULL)\n");
				return 0;
			}
		
			memcpy(&(mem->memory[mem->size]), contents, realsize);
			mem->size += realsize;
			mem->memory[mem->size] = 0;
		
			return realsize;
		}
		size_t curlWriteDataFile(void *ptr, size_t size, size_t nmemb, void *stream){
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
		void disableSSL(){
			curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
		}
		void initDownload(){
			#if PLATFORM == PLAT_VITA
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

			// Loads the certificate for https stuff. If the certificate file does not exist, allow insecure connections
			FixPath(CONSTANTCERTFILELOCATION,tempPathFixBuffer,TYPE_EMBEDDED);
			if (!checkFileExist(tempPathFixBuffer)){
				printf("%s not found! Will allow insecure connections!",tempPathFixBuffer);
				curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
			}else{
				curl_easy_setopt(curl_handle, CURLOPT_CAINFO, tempPathFixBuffer);
			}
			curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		}
	#elif DOWNLOADTYPE == DOWNLOAD_VITA
		
	#endif

#endif