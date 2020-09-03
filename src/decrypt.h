struct decstate;
void initdecstate(struct decstate* d, const char* _inFilename, const unsigned char* _password, int _passwordLen);
void decryptmore(struct decstate* d, unsigned char* _retBuff, int _desiredBytes);
void freedecryptstate(struct decstate* d);
uint64_t dread64(struct decstate* d); // that's d-read.
struct decstate* mallocdecstate();
char decryptioneof(struct decstate* d);
