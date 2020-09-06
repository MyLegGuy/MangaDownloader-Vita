#include <stdio.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <string.h>
#include <goodbrew/config.h>
#define DECRYPTINBYTES 16000
struct decstate{
	const EVP_CIPHER* cipher;
	EVP_CIPHER_CTX* ctx;
	FILE* fp;
	char* decBuff;
	int decBuffOff; // offset when giving out more
	int decBuffLeft; // how much is left to give out.
	char isDone;
	char isFreed;
	int readCounter;
};
struct decstateplus{
	struct decstate d;
	int curPage;
	int totalPages;
};
void myZeroBuff( void *v, size_t n ){
	#ifdef __GNU_LIBRARY__
	explicit_bzero(v,n);
	#else
	volatile unsigned char *p = v;
	while( n-- ) *p++ = 0;
	#endif
}
#include "cryptshared.h"
#if GBPLAT == GB_VITA
uint32_t le32toh(uint32_t a){
	return a;
}
uint64_t le64toh(uint64_t a){
	return a;
}
#endif
/* struct decstate* mallocdecstate(){ */
/* 	return malloc(sizeof(struct decstate)); */
/* } */
///
struct decstate* mallocdecstateplus(){
	struct decstateplus* _ret = malloc(sizeof(struct decstateplus));
	_ret->curPage=0;
	return (struct decstate*)_ret;
}
char decstateplusImGoingToNextPage(struct decstate* d){
	struct decstateplus* p = (struct decstateplus*)d;
	if ((p->curPage++)==p->totalPages){
		return 1;
	}
	return 0;
}
void decstateplusSetTotalPages(struct decstate* d, int val){
	struct decstateplus* p = (struct decstateplus*)d;
	p->totalPages=val;
}
void decstateResetCounter(struct decstate* d){
	d->readCounter=0;
}
int decstateReadCount(struct decstate* d){
	return d->readCounter;
}
///
void handleErrors(){
	fprintf(stderr,"oh");
	exit(1);
}
void safefread(void* ptr, size_t size, size_t nmemb, FILE* stream){
	if (fread(ptr,size,nmemb,stream)!=nmemb){
		perror("fread");
		exit(1);
	}
}
void freadexpected(FILE* fp, char* _expected){
	int _len = strlen(_expected);
	char _buff[_len];
	safefread(_buff,1,_len,fp);
	if (memcmp(_expected,_buff,_len)!=0){
		fprintf(stderr,"read bad data. expected %s\n",_expected);
		exit(1);
	}
}
uint32_t read32(FILE* fp){
	uint32_t _read;
	safefread(&_read,1,4,fp);
	return le32toh(_read);
}
uint64_t read64(FILE* fp){
	uint64_t _read;
	safefread(&_read,1,8,fp);
	return le64toh(_read);
}
void freedecryptstate(struct decstate* d){
	if (!d->isFreed){
		fclose(d->fp);
		free(d->decBuff);
		EVP_CIPHER_CTX_free(d->ctx);
		d->isFreed=1;
	}
}
void decryptmore(struct decstate* d, unsigned char* _retBuff, int _desiredBytes){
top:
	if (d->decBuffLeft>0){
		int _numCopy=d->decBuffLeft>_desiredBytes ? _desiredBytes : d->decBuffLeft;
		memcpy(_retBuff,d->decBuff+d->decBuffOff,_numCopy);
		_desiredBytes-=_numCopy;
		_retBuff+=_numCopy;
		d->decBuffLeft-=_numCopy;
		d->decBuffOff+=_numCopy;
		d->readCounter+=_numCopy;
	}
	if (_desiredBytes>0){
		d->decBuffOff=0;
		char _justReadBuff[DECRYPTINBYTES];
		int _numRead = fread(_justReadBuff,1,DECRYPTINBYTES,d->fp);
		if (_numRead!=DECRYPTINBYTES){
			if (feof(d->fp)){
				if (_numRead==0){
					if (!d->isDone){
						d->isDone=1;
						if(EVP_DecryptFinal_ex(d->ctx,d->decBuff,&d->decBuffLeft)!=1){
							printf("err\n");
						}
						goto skipregulardec;
					}else{
						myZeroBuff(_retBuff,_desiredBytes);
						return;
					}
				}
			}else{
				perror("fread for dec");
				exit(1);
			}
		}
		if(EVP_DecryptUpdate(d->ctx, d->decBuff, &d->decBuffLeft, _justReadBuff, _numRead)!=1){
			handleErrors();
		}
	skipregulardec:
		goto top;
	}
}
uint64_t dread64(struct decstate* d){
	uint64_t _ret;
	decryptmore(d,(unsigned char*)&_ret,8);
	_ret=le64toh(_ret);
	return _ret;
}
void initdecstate(struct decstate* d, const char* _inFilename, const unsigned char* _password, int _passwordLen){
	d->isFreed=0;
	if (!(d->fp = fopen(_inFilename,"rb"))){
		perror(NULL);
		exit(1);
	}
	fgetc(d->fp); // skip 01
	fgetc(d->fp); // skip ENCVERSIONNUM
	freadexpected(d->fp,"CMIS");
	d->cipher = EVP_get_cipherbynid(read32(d->fp));
	const EVP_MD* _hash = EVP_get_digestbynid(read32(d->fp));
	long _iterations = read64(d->fp);
	int _saltLen = fgetc(d->fp);
	char _salt[_saltLen];
	safefread(_salt,1,_saltLen,d->fp);

	unsigned char* _key;
	unsigned char* _iv;
	getkeyandiv(_password,_passwordLen, _salt, _saltLen, d->cipher, &_key, &_iv, _hash,_iterations);

	d->decBuffOff=0;
	d->decBuffLeft=0;
	d->decBuff=malloc(DECRYPTINBYTES+EVP_CIPHER_block_size(d->cipher));
	d->isDone=0;
	
	/* Create and initialise the context */
	if(!(d->ctx = EVP_CIPHER_CTX_new())){
		handleErrors();
	}
	/*
	 * Initialise the decryption operation. IMPORTANT - ensure you use a key
	 * and IV size appropriate for your cipher
	 * In this example we are using 256 bit AES (i.e. a 256 bit key). The
	 * IV size for *most* modes is the same as the block size. For AES this
	 * is 128 bits
	 */
	if(EVP_DecryptInit_ex(d->ctx, d->cipher, NULL, _key, _iv)!=1){
		handleErrors();
	}
}
char decryptioneof(struct decstate* d){
	return d->isDone && d->decBuffLeft==0;
}
