/* license for this file: cc0 */
// TODO - https://github.com/libvips/libvips to resize images
#include <stdio.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <string.h>
#include <dirent.h>

// enc settings
//#define MYITERATIONS 1000000 // good for pc. very long on vita.
//#define MYITERATIONS 100000 // ~12 seconds on vita with sha256 to load initial page
//#define MYITERATIONS 70000 // ~10 seconds
//#define MYITERATIONS 30000 // ~6 seconds
//#define MYITERATIONS 20000 // ~4 seconds (with about 2 being for the page)
//#define MYITERATIONS 25000 // ~5 seconds, (~3.5 overclocked)
#define MYITERATIONS 30000 // ~4.5 seconds overclocked
#define MYHASH EVP_sha256()
#define MYDEFAULTCIPHER EVP_aes_256_cbc()
#define DEFAULTSALTLEN 16
#define ENCVERSIONNUM 1 // for the file foramt that stores the encryption settings

/*
DECRYPTED FILE FORMAT:
"HMANGA" (ASCII)
uint8_t version
char* name (null terminated)
int numPages;
for (int i=0;i<numPages;++i){
	long _filesize;
	<file data goes here>
}
*/
#define MAGICSTRING "HMANGA"
#define HMANGAVERSION 1
#define FILENAMEEXT ".h"
#define DOENCRYPTNONSENSE 1

struct cryptstate{
	const EVP_CIPHER* cipher;
	FILE* fp;
	EVP_CIPHER_CTX* ctx;
};
static void myZeroBuff( void *v, size_t n ){
	#ifdef __GNU_LIBRARY__
	explicit_bzero(v,n);
	#else
	volatile unsigned char *p = v;
	while( n-- ) *p++ = 0;
	#endif
}
void safefwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream){
	int _wrote=fwrite(ptr,size,nmemb,stream);
	if (_wrote!=nmemb){
		perror("fwrite");
		exit(1);
	}
}
void safefputc(int c, FILE* stream){
	if (fputc(c,stream)==EOF){
		perror("fputc");
		exit(1);
	}
}
void safefread(void* ptr, size_t size, size_t nmemb, FILE* stream){
	if (fread(ptr,size,nmemb,stream)!=nmemb){
		perror("fread");
		exit(1);
	}
}
void safefseek(FILE* stream, long offset, int whence){
	if (fseek(stream,offset,whence)){
		perror("fseek");
		exit(1);
	}
}
void fileToBuff(const char* _filename, unsigned char** _retBuff, int* _retLen){
	FILE* fp = fopen(_filename, "rb");
	if (!fp){
		perror("fopen");
		fprintf(stderr,"%s\n",_filename);
		exit(1);
	}
	safefseek(fp, 0, SEEK_END);
	*_retLen = (int)ftell(fp);
	safefseek(fp, 0, SEEK_SET);
	*_retBuff = malloc(*_retLen);
	safefread(*_retBuff, *_retLen, 1, fp);
	if (fclose(fp)){
		perror("fclose");
		exit(1);
	}
}
void handleErrors(){
	printf("error\n");
	exit(1);
}
#include "../src/cryptshared.h"
void encryptsomemoretempbuff(struct cryptstate* _in, const char* _plain, int _plainLen, char* _encBuff){
	int _numEncBytes;
	if(EVP_EncryptUpdate(_in->ctx, _encBuff, &_numEncBytes, _plain, _plainLen)!=1){
		handleErrors();
	}
	if (_numEncBytes!=0){
		safefwrite(_encBuff,1,_numEncBytes,_in->fp);
	}
}
void encryptsomemore(struct cryptstate* _in, const char* _plain, int _plainLen){
	char _encBuff[_plainLen+EVP_CIPHER_block_size(_in->cipher)-1]; // correct buffer size according ot the documentation
	encryptsomemoretempbuff(_in,_plain,_plainLen,_encBuff);
}
void efwrite(const void *ptr, size_t size, size_t nmemb, struct cryptstate* _in){
	encryptsomemore(_in,ptr,size*nmemb);
}
void efputc(int c, struct cryptstate* _in){
	unsigned char b = c;
	efwrite(&b,1,1,_in);
}
void finishcryptstate(struct cryptstate* _in){
	/*
	 * Finalise the encryption. Further ciphertext bytes may be written at
	 * this stage.
	 */
	char _encBuff[EVP_CIPHER_block_size(_in->cipher)]; // correct buffer size according ot the documentation
	int _numEncBytes;
	if(EVP_EncryptFinal_ex(_in->ctx,_encBuff,&_numEncBytes)!=1){
		handleErrors();
	}
	if (_numEncBytes!=0){
		safefwrite(_encBuff,1,_numEncBytes,_in->fp);
	}
	//
	EVP_CIPHER_CTX_free(_in->ctx);
	if (fclose(_in->fp)){
		perror("fclose");
		exit(1);
	}
}
void write32(FILE* fp, uint32_t n){
	n = htole32(n);
	safefwrite(&n,1,4,fp);
}
void write64(FILE* fp, uint64_t n){
	n = htole64(n);
	safefwrite(&n,1,8,fp);
}
void ewrite32(struct cryptstate* fp, uint32_t n){
	n = htole32(n);
	efwrite(&n,1,4,fp);
}
void ewrite64(struct cryptstate* fp, uint64_t n){
	n = htole64(n);
	efwrite(&n,1,8,fp);
}
void initcryptstate(struct cryptstate* _ret, const char* _password, int _passwordLen, const char* _outFilename){
	_ret->cipher=MYDEFAULTCIPHER;
	int _saltLen=DEFAULTSALTLEN;
	unsigned char _salt[_saltLen];
	if (RAND_bytes(_salt,DEFAULTSALTLEN)!=1){
		fprintf(stderr,"failed to generate bytes for salt\n");
		exit(1);
	}
	unsigned char* _key;
	unsigned char* _iv;
	getkeyandiv(_password,_passwordLen,_salt,_saltLen,_ret->cipher,&_key,&_iv,MYHASH,MYITERATIONS);
	//
	if (!(_ret->fp=fopen(_outFilename,"wb"))){
		handleErrors();
	}

	// the magic hints at the order of the metadata
	if (fputc(01,_ret->fp)==EOF || fputc(ENCVERSIONNUM,_ret->fp)==EOF || fputc('C',_ret->fp)==EOF || fputc('M',_ret->fp)==EOF || fputc('I',_ret->fp)==EOF || fputc('S',_ret->fp)==EOF){
		perror(NULL);
		exit(1);
	}
	write32(_ret->fp, EVP_CIPHER_nid(_ret->cipher)); // and EVP_get_cipherbynid()
	write32(_ret->fp, EVP_MD_type(MYHASH)); // and EVP_get_digestbynid()
	write64(_ret->fp, MYITERATIONS);
	if (fputc(_saltLen,_ret->fp)==EOF){
		perror(NULL);
		exit(1);
	}
	safefwrite(_salt,1,_saltLen,_ret->fp);
	//
	if(!(_ret->ctx = EVP_CIPHER_CTX_new())){
		handleErrors();
	}
	/*
	 * Initialise the encryption operation. IMPORTANT - ensure you use a key
	 * and IV size appropriate for your cipher
	 * In this example we are using 256 bit AES (i.e. a 256 bit key). The
	 * IV size for *most* modes is the same as the block size. For AES this
	 * is 128 bits
	 */
	if(EVP_EncryptInit_ex(_ret->ctx, _ret->cipher, NULL, _key, _iv)!=1){
		handleErrors();
	}
	freekeyandiv(_ret->cipher,_key,_iv);
}
static int compareStringsInsensitive(const void* first, const void* second){
	return strcasecmp(*((char**)first),*((char**)second));
}
void getImageList(const char* _dirPath, char*** _retList, int* _retLen){
	DIR* _curDir = opendir(_dirPath);
	if (!_curDir){
		perror("opendir");
		fprintf(stderr,"%s\n",_dirPath);
		return;
	}
	char** _fileList=NULL;
	int _fileListSize=0;
	int _fileListUsed=0;
	while(1){
		errno=0;
		struct dirent* _curEntry;
		if (!(_curEntry=readdir(_curDir))){
			if (errno){
				perror("readdir");
				exit(1);
			}
			break;
		}
		if (!(_curEntry->d_type==DT_LNK || _curEntry->d_type==DT_REG)){
			continue;
		}
		// check ext
		{
			const char* _extStart = strrchr(_curEntry->d_name,'.');
			if (!_extStart){
				continue;
			}
			if (!(strcmp(_extStart,".jpg")==0 || strcmp(_extStart,".jpeg")==0 || strcmp(_extStart,".png")==0)){
				continue;
			}
		}
		if (_fileListUsed>=_fileListSize){
			_fileListSize=(_fileListSize+1)*2;
			_fileList=realloc(_fileList,sizeof(char*)*_fileListSize);
		}
		_fileList[_fileListUsed++]=strdup(_curEntry->d_name);
	}
	if (closedir(_curDir)){
		perror("closedir");
		exit(1);
	}
	qsort(_fileList,_fileListUsed,sizeof(char*),compareStringsInsensitive);
	*_retList=_fileList;
	*_retLen=_fileListUsed;
}
void writeName(const char* _inDir, struct cryptstate* fp){
	const char* _nameStart;
	int _dirLen = strlen(_inDir);
	if (_inDir[_dirLen-1]=='/' && _dirLen!=1){
		_nameStart=NULL;
		for (int i=_dirLen-2;i>0;--i){
			if (_inDir[i]=='/'){
				_nameStart=&(_inDir[i]);
				break;
			}
		}
	}else{
		_nameStart = strrchr(_inDir,'/');
	}
	const char* _writeStr = _nameStart ? _nameStart+1 : _inDir;
	efwrite(_writeStr,1,strlen(_writeStr)+1,fp);
}
#define COPYBUFFSIZE 16000
void copyfile(FILE* _in, struct cryptstate* _out){
	static char _buff[COPYBUFFSIZE]; // not thread safe
	while(!feof(_in)){
		size_t _bytesRead = fread(_buff,1,1,_in);
		efwrite(_buff,1,_bytesRead,_out);
	}
}
void writeFile(const char* _fullPath, struct cryptstate* _out){
	FILE* in = fopen(_fullPath,"rb");
	if (!in){
		perror(_fullPath);
		exit(1);
	}
	//
	fseek(in, 0, SEEK_END);
	long _size = ftell(in);
	fseek(in, 0, SEEK_SET);
	//
	ewrite64(_out,_size);
	copyfile(in, _out);
	//
	if (fclose(in)){
		perror("fclose");
		exit(1);
	}
}
void packdir(const char* _inDir, const char* _outFilename, unsigned char* _password, int _passwordLen){
	char** _filenameList;
	int _listLen;
	getImageList(_inDir,&_filenameList,&_listLen);

	struct cryptstate fp;
	initcryptstate(&fp,_password,_passwordLen,_outFilename);

	efwrite(MAGICSTRING,1,strlen(MAGICSTRING),&fp);
	efputc(HMANGAVERSION,&fp);
	writeName(_inDir,&fp);
	ewrite32(&fp,_listLen);
	int _dirLen=strlen(_inDir);
	for (int i=0;i<_listLen;++i){
		int _nameLen=strlen(_filenameList[i]);
		char _fullPath[_dirLen+1+_nameLen+1];
		memcpy(_fullPath,_inDir,_dirLen);
		_fullPath[_dirLen]='/';
		strcpy(_fullPath+_dirLen+1,_filenameList[i]);

		writeFile(_fullPath,&fp);
	}
	#if DOENCRYPTNONSENSE
	{
		// write a bunch of encrypted zeroes.
		uint32_t rng=0;
		if (RAND_bytes((unsigned char*)&rng,3)!=1){
			fprintf(stderr,"failed to generate bytes for padding amount\n");
			exit(1);
		}
		rng=le32toh(rng);
		rng=(uint32_t)rng<<10; // trim it to 22 bits only. max is 4mb, 4194304
		rng=(uint32_t)rng>>10;
		// do it
		int _maxNumPut=200;
		char _encBuff[_maxNumPut+EVP_CIPHER_block_size(fp.cipher)-1];
		char _zeroes[_maxNumPut];
		memset(_zeroes,0,_maxNumPut);
		while(rng>0){
			int _numPut=200;
			if (rng<_numPut){
				_numPut=rng;
			}
			rng-=_numPut;
			encryptsomemoretempbuff(&fp,_zeroes,_numPut,_encBuff);
		}
	}
	#endif
	finishcryptstate(&fp);
}
#define FILENAMEBYTES 5
char* genFilename(){
	unsigned char _bytes[FILENAMEBYTES];
	if (RAND_bytes(_bytes,FILENAMEBYTES)!=1){
		fprintf(stderr,"failed to generate bytes for filename\n");
		exit(1);
	}
	char* _ret = malloc(FILENAMEBYTES*2+1);
	int i;
	for (i=0;i<FILENAMEBYTES;++i){
		sprintf(&(_ret[i*2]),"%02X",(unsigned char)(_bytes[i]));
	}
	_ret[i*2]='\0';
	return _ret;
}
void highDoDir(const char* _inpath, const char* _outDir, unsigned char* _passwordBuff, int _passwordLen){
	char* _fullOutPath;
	{
		char* _outFilename=genFilename();
		int _nameLen = strlen(_outFilename);
		int _outDirLen = strlen(_outDir);
		_fullOutPath=malloc(_outDirLen+1+_nameLen+strlen(FILENAMEEXT)+1);
		memcpy(_fullOutPath,_outDir,_outDirLen);
		_fullOutPath[_outDirLen]='/';
		memcpy(_fullOutPath+_outDirLen+1,_outFilename,_nameLen);
		_fullOutPath[_outDirLen+1+_nameLen]='\0';
		strcat(_fullOutPath,FILENAMEEXT);
		free(_outFilename);
	}
	printf("%s -> %s\n",_inpath,_fullOutPath);
	packdir(_inpath,_fullOutPath,_passwordBuff,_passwordLen);
	free(_fullOutPath);
}
int main(int argc, char** args){
	if (argc<=1){
		fprintf(stderr,"%s <password file> <out dir> [-i <indirlist>] [in dir 1] [in dir 2] [in dir ...]\n",argc>0 ? args[0] : "hmangacrypt");
		return 1;
	}
	unsigned char* _passwordBuff;
	int _passwordLen;
	fileToBuff(args[1],&_passwordBuff,&_passwordLen);
	//
	const char* _outDir = args[2];
	for (int i=3;i<argc;++i){
		if (strcmp(args[i],"-i")==0){
			FILE* fp = fopen(args[i+1],"rb");
			if (!fp){
				perror(args[i]);
				continue;
			}
			while(1){
				char* _line=NULL;
				size_t _lineLen=0;
				errno=0;
				if (getline(&_line,&_lineLen,fp)==-1){
					if (errno!=0){
						perror("getline");
						exit(1);
					}
					break;
				}
				{
					int _l = strlen(_line);
					if (_l==0){
						free(_line);
						continue;
					}
					if (_line[_l-1]==0x0A){
						_line[_l-1]='\0';
					}
				}
				highDoDir(_line,_outDir,_passwordBuff,_passwordLen);
				free(_line);
			}
			if (fclose(fp)){
				perror(NULL);
			}
			++i;
		}else{
			highDoDir(args[i],_outDir,_passwordBuff,_passwordLen);
		}
	}
	myZeroBuff(_passwordBuff,_passwordLen);
	free(_passwordBuff);
}
