struct decstate;
void initdecstate(struct decstate* d, const char* _inFilename, const unsigned char* _password, int _passwordLen);
void decryptmore(struct decstate* d, unsigned char* _retBuff, int _desiredBytes);
void freedecryptstate(struct decstate* d);
uint64_t dread64(struct decstate* d); // that's d-read.
struct decstate* mallocdecstateplus();
char decryptioneof(struct decstate* d);
void myZeroBuff( void *v, size_t n );
char decstateplusImGoingToNextPage(struct decstate* d);
void decstateplusSetTotalPages(struct decstate* d, int val);
int decstateReadCount(struct decstate* d);
void decstateResetCounter(struct decstate* d);
