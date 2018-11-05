#ifndef MAINHINCLUDED
#define MAINHINCLUDED
char popupMessage(const char* _tempMsg, char _waitForAButton, char _isQuestion);
char* getFileExtention(char* _filename, int _extentionLength);
void WriteIntToDebugFile(int a);
void WriteToDebugFile(const char* stuff);
void alphabetizeList(char** _passedList,int _totalFileListLength);
#endif