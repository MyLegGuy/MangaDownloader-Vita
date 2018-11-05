#ifndef NETCODEHEADERINCLUDED
#define NETCODEHEADERINCLUDED
void cleanupNetCode();
void disableSSLVerification();
void downloadEnableDebugInfo();
char* downloadGetLastRedirect();
void downloadSetRedirects(char _isEnabled);
void downloadToFile(const char* passedUrl, const char* passedFilename);
void downloadWebpageData(const char* url, char** _toStoreWebpageData, size_t* _toStoreSize);
void initDownload(char* _certLocation);
void setReferer(const char* _newReferer);
void setUserAgent(const char* _agentName);
#endif