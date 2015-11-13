/*
**	TEA（Tiny Encryption Algorithm）
*/

#pragma once

#include <string.h>
#include <string>

#ifdef CLASS_EXPORT
#define CRYPTO_API __declspec(dllexport)
#elif defined(CLASS_IMPORT)
#define CRYPTO_API __declspec(dllimport)
#else
#define CRYPTO_API
#endif


class CRYPTO_API Crypto
{
public:
	Crypto(unsigned char* tkey) :_Tkey(tkey), mlength(0), encryptedout(NULL), decryptedout(NULL), header(1)
    {
        memset(plain, 0, 8);
        memset(prePlain, 0, 8);
    }
	~Crypto();
    void TEncrypt(unsigned char* in, int offset, int len);
    int  TDecrypt(unsigned char* in, int offset, int len);
    unsigned char *GetEncryptedBytes(void);
    unsigned char *GetDecryptedBytes(void);
    int GetEncryptedSize(void);
    int GetDecryptedSize(void);

private:
    /*
    ** 加密8字节
    */
    void encrypt8Bytes(void) ;

    /*
    ** 加密一个8字节块
    ** @param in   明文字节数组
    ** @param out1 密文字节数组
    */
    void encipher(unsigned int* in, unsigned int* out1);

    /*
    ** 解密8个字节
    ** @param in  	密文字节数组
    ** @param offset 从何处开始解密
    ** @param len  	密文的长度
    ** @return 		1表示解密成功
    */
	int decrypt8Bytes(unsigned char* in, int offset, int len);

    /*
    ** 解密从offset开始的8字节密文
    ** @param in 		密文
    ** @param offset 	密文开始位置
    */
    void decipher(unsigned int* in);

    int _rand(void);

	// Tea加密解密的密钥
    unsigned char* _Tkey;

    // 解密后的长度
    int mlength;

    // 指向当前的明文块
    unsigned char plain[8];

    // 这指向前面一个明文块
    unsigned char prePlain[8];

    // 输出的密文
    unsigned char* encryptedout;

    // 输出的明文
    unsigned char* decryptedout;

    // 当前加密的密文位置和上一次加密的密文块位置，他们相差8
    int curCrypt, preCrypt;

    // 当前处理的加密解密块的位置
    int pos;

    // 填充数
    int padding;

    // 用于加密时，表示当前是否是第一个8字节块，因为加密算法是反馈的
    // 但是最开始的8个字节没有反馈可用，所有需要标明这种情况
    short header;

    // 这个表示当前解密开始的位置，之所以要这么一个变量是为了避免当解密到最后时
    // 后面已经没有数据，这时候就会出错，这个变量就是用来判断这种情况免得出错
    int contextStart;
};
