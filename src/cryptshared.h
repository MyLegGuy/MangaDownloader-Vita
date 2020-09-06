void getkeyandiv(const unsigned char* _password, int _passwordLen, const unsigned char* _salt, int _saltLen, const EVP_CIPHER *cipher, unsigned char** _retKey, unsigned char** _retiv, const EVP_MD *digest, long _iterations){
	// absolute god: https://stackoverflow.com/a/62240645/5022354
	int iklen = EVP_CIPHER_key_length(cipher);
	int ivlen = EVP_CIPHER_iv_length(cipher);
	char keyivpair[iklen+ivlen];
	PKCS5_PBKDF2_HMAC(_password, _passwordLen, _salt, _saltLen, _iterations, digest, iklen + ivlen, keyivpair);
	*_retiv = malloc(ivlen);
	*_retKey = malloc(iklen);
	memcpy(*_retKey, keyivpair, iklen);
	memcpy(*_retiv, keyivpair + iklen, ivlen);
	myZeroBuff(keyivpair,iklen+ivlen);
}
void freekeyandiv(const EVP_CIPHER *cipher, unsigned char* _key, unsigned char* _iv){
	int iklen = EVP_CIPHER_key_length(cipher);
	int ivlen = EVP_CIPHER_iv_length(cipher);
	myZeroBuff(_key,iklen);
	myZeroBuff(_iv,ivlen);
	free(_key);
	free(_iv);
}
