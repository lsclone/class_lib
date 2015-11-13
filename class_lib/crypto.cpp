#include "crypto.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#ifdef WIN32
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#endif

using namespace std;

/* 运算符'^': 按位异或 */

Crypto::~Crypto()
{
    if(encryptedout != NULL)
    {
        delete[] encryptedout;
        encryptedout = NULL;
    }
    if(decryptedout != NULL)
    {
        delete[] decryptedout;
        decryptedout = NULL;
    }
}

void Crypto::TEncrypt(unsigned char* in, int offset, int len)
{
    pos = 1;
    padding = 0;
    curCrypt = preCrypt = 0;
    header = 1;
    // 计算头部填充字节数
    pos = (len + 0x0A) % 8;
    if(pos)
        pos = 8 - pos;
    // 计算输出的密文长度
    if(encryptedout != NULL)
    {
        free(encryptedout);
        encryptedout = NULL;
    }
    encryptedout = ( unsigned char* )malloc((len + pos + 10)*sizeof(unsigned char));//new unsigned char[(len + pos + 10)*sizeof(unsigned char)];
    memset(encryptedout,0,(len + pos + 10)*sizeof(unsigned char));
    // 这里的操作把pos存到了plain的第一个字节里面
    // 0xF8后面三位是空的，正好留给pos，因为pos是0到7的值，表示文本开始的字节位置
    plain[0] = (unsigned char)((_rand() & 0xF8) | pos);
    // 这里用随机产生的数填充plain[1]到plain[pos]之间的内容
    int i = 1;
    for(; i <= pos; i++)
        plain[i] = (unsigned char)(_rand() & 0xFF);
    pos++;
    i = 0;
    for(; i < 8; i++)
        prePlain[i] = 0x0;
    // 继续填充2个字节的随机数，这个过程中如果满了8字节就加密之
    padding = 1;
    while(padding <= 2)
    {
        if(pos < 8)
        {
            plain[pos++] = (unsigned char)(_rand() & 0xFF);
            padding++;
        }
        if(pos == 8)
            encrypt8Bytes();
    }
    // 头部填充完了，这里开始填真正的明文了，也是满了8字节就加密，一直到明文读完
    i = offset;
    while(len > 0)
    {
        if(pos < 8)
        {
            plain[pos++] = in[i++];
            len--;
        }
        if(pos == 8)
            encrypt8Bytes();
    }
    // 最后填上0，以保证是8字节的倍数
    padding = 1;
    while(padding <= 7)
    {
        if(pos < 8)
        {
            plain[pos++] = 0x0;
            padding++;
        }
        if(pos == 8)
            encrypt8Bytes();
    }
}

int  Crypto::TDecrypt(unsigned char* in, int offset, int len)
{
    if((len % 8 != 0) || (len < 16))
        return 1;
    curCrypt = preCrypt = 0;
    // 得到消息的头部，关键是得到真正明文开始的位置，这个信息存在第一个字节里面，所以其用解密得到的第一个字节与7做与
    decipher((unsigned int*)in);
    pos = prePlain[0] & 0x7;
    // 得到真正明文的长度
    int count = mlength = len - pos - 10;
    // 如果明文长度小于0，那肯定是出错了，比如传输错误之类的，返回
    if(count < 0)
        return 1;

    unsigned char* p = NULL;

    if(decryptedout != NULL)
    {
        free(decryptedout);
        decryptedout = NULL;
    }
    // 通过了上面的代码，密文应该是没有问题了，我们分配输出缓冲区
    decryptedout = ( unsigned char* )malloc(count*sizeof(unsigned char));
    memset(decryptedout,0,count*sizeof(unsigned char));
    // 设置preCrypt的位置等于0，注意目前的preCrypt位置是指向m的，因为java没有指针，所以我们在后面要控制当前密文buf的引用
    preCrypt = 0;
    // 当前的密文位置，为什么是8不是0呢？注意前面我们已经解密了头部信息了，现在当然该8了
    curCrypt = 8;
    // 自然这个也是8
    contextStart = 8;
    // 加1，和加密算法是对应的
    pos++;
    // 开始跳过头部，如果在这个过程中满了8字节，则解密下一块
    // 因为是解密下一块，所以我们有一个语句 m = in，下一块当然有preCrypt了，我们不再用m了
    // 但是如果不满8，这说明了什么？说明了头8个字节的密文是包含了明文信息的，当然还是要用m把明文弄出来
    // 所以，很显然，满了8的话，说明了头8个字节的密文除了一个长度信息有用之外，其他都是无用的填充
    padding = 1;
    while(padding <= 2) {
        if(pos < 8) {
            pos++;
            padding++;
        }
        if(pos == 8) {
            p = in;
            decrypt8Bytes(in, offset, len);
        }
    }
    // 这里是解密的重要阶段，这个时候头部的填充都已经跳过了，开始解密
    // 注意如果上面一个while没有满8，这里第一个if里面用的就是原始的m，否则这个m就是in了
    int j = 0;
    char m[8] = {0};
    while(count != 0) {
        if(pos < 8)
        {
            if(p==NULL)
                decryptedout[j] = (char)(m[offset + preCrypt + pos] ^ prePlain[pos]);
            else
                decryptedout[j] = (char)(p[offset + preCrypt + pos] ^ prePlain[pos]);
            j++;
            count--;
            pos++;
        }
        if(pos == 8)
        {
            p = in;
            preCrypt = curCrypt - 8;
            decrypt8Bytes(in, offset, len);
        }
    }
    // 最后的解密部分，上面一个while已经把明文都解出来了，就剩下尾部的填充了，应该全是0
    // 所以这里有检查是否解密了之后是不是0，如果不是的话那肯定出错了，返回null
    for(padding = 1; padding < 8; padding++)
    {
        if(pos < 8)
        {
            char t = p[offset + preCrypt + pos];
            char t1 = prePlain[pos];
            int k = (t^t1);
            if(k != 0 )
            {
                return 1;
            }
            pos++;
        }
        if(pos == 8)
        {
            p = in;
            preCrypt = curCrypt;
            decrypt8Bytes(in, offset, len);
        }
    }
    return 0;
}

unsigned char *Crypto::GetEncryptedBytes(void)
{
    return encryptedout;
}

unsigned char *Crypto::GetDecryptedBytes(void)
{
    return decryptedout;
}

int Crypto::GetEncryptedSize(void)
{
    return curCrypt;
}

int Crypto::GetDecryptedSize(void)
{
    return mlength;
}

void Crypto::encrypt8Bytes()
{
    // 这部分完成我上面所说的 plain ^ preCrypt，注意这里判断了是不是第一个8字节块，如果是的话，那个prePlain就当作preCrypt用
    for(pos = 0; pos < 8; pos++)
    {
        if(header)
            plain[pos] ^= prePlain[pos];
        else
            plain[pos] ^= encryptedout[preCrypt + pos];
    }
    // 这个完成我上面说的 f(plain ^ preCrypt)
    unsigned char crypted[8]={0};
    encipher((unsigned int*)plain, (unsigned int*)crypted);
    memcpy(encryptedout+curCrypt,crypted,8);
    // 这个完成了 f(plain ^ preCrypt) ^ prePlain，ok，下面拷贝一下就行了
    for(pos = 0; pos < 8; pos++)
        encryptedout[curCrypt + pos] ^= prePlain[pos];

    memcpy(prePlain,plain,8);
    // 完成了加密，现在是调整crypt，preCrypt等等东西的时候了
    preCrypt = curCrypt;
    curCrypt += 8;
    pos = 0;
    header = 0;
}

void Crypto::encipher(unsigned int* in, unsigned int* out1)
{
    // 迭代次数，16次
    int loop = 0x10;

    unsigned int y = ntohl(in[0]);
    unsigned int z = ntohl(in[1]);
    unsigned int a = ntohl(((unsigned int*)_Tkey)[0]);	//密钥的低4位字节
    unsigned int b = ntohl(((unsigned int*)_Tkey)[1]);	//
    unsigned int c = ntohl(((unsigned int*)_Tkey)[2]);	//
    unsigned int d = ntohl(((unsigned int*)_Tkey)[3]);	//密钥的高4位字节

    // 这是算法的一些控制变量，为什么delta是0x9E3779B9呢？
    // 这个数是TEA算法的delta，实际是就是(sqr(5) - 1) * 2^31 (根号5，减1，再乘2的31次方)
    unsigned int sum = 0;
    unsigned int delta = 0x9E3779B9;
    delta &= 0xFFFFFFFFL;

    // 开始迭代了，乱七八糟的，我也看不懂，反正和DES之类的差不多，都是这样倒来倒去
    while ((loop--) > 0)
    {
        sum += delta;
        sum &= 0xFFFFFFFFL;
        y += ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
        y &= 0xFFFFFFFFL;
        z += ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
        z &= 0xFFFFFFFFL;
    }
    // 最后，我们输出密文，因为我用的long，所以需要强制转换一下变成int

    out1[0]=ntohl(y);
    out1[1]=ntohl(z);
}

int Crypto::decrypt8Bytes(unsigned char* in, int offset, int len)
{
    // 这里第一步就是判断后面还有没有数据，没有就返回，如果有，就执行 crypt ^ prePlain
    for(pos = 0; pos < 8; pos++) {
        if(contextStart + pos >= len)
            return 1;
        prePlain[pos] ^= in[offset + curCrypt + pos];
    }

    // 好，这里执行到了 d(crypt ^ prePlain)
    decipher((unsigned int*)prePlain);

    // 解密完成，最后一步好像没做？
    // 这里最后一步放到decrypt里面去做了，因为解密的步骤有点不太一样
    // 调整这些变量的值先
    contextStart += 8;
    curCrypt += 8;
    pos = 0;
    return 1;
}

void Crypto::decipher(unsigned int* in)
{
    // 迭代次数，16次
    int loop = 0x10;
    // 得到密文和密钥的各个部分，注意java没有无符号类型，所以为了表示一个无符号的整数
    // 我们用了int，这个int的前32位是全0的，我们通过这种方式模拟无符号整数，后面用到的long也都是一样的
    // 而且为了保证前32位为0，需要和0xFFFFFFFF做一下位与
    unsigned int y = ntohl(in[0]);
    unsigned int z = ntohl(in[1]);
    unsigned int a = ntohl(((unsigned int*)_Tkey)[0]);	//密钥的低4位字节
    unsigned int b = ntohl(((unsigned int*)_Tkey)[1]);	//
    unsigned int c = ntohl(((unsigned int*)_Tkey)[2]);	//
    unsigned int d = ntohl(((unsigned int*)_Tkey)[3]);	//密钥的高4位字节
    // 算法的一些控制变量，sum在这里也有数了，这个sum和迭代次数有关系
    // 因为delta是这么多，所以sum如果是这么多的话，迭代的时候减减减，减16次，最后
    // 得到0。反正这就是为了得到和加密时相反顺序的控制变量，这样才能解密呀～～
    int sum = 0xE3779B90;
    sum &= 0xFFFFFFFFL;
    int delta = 0x9E3779B9;
    delta &= 0xFFFFFFFFL;
    // 迭代开始了， @_@
    while(loop-- > 0)
    {
        z -= ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
        z &= 0xFFFFFFFFL;
        y -= ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
        y &= 0xFFFFFFFFL;
        sum -= delta;
        sum &= 0xFFFFFFFFL;
    }
    ((unsigned int*)prePlain)[0]=ntohl(y);
    ((unsigned int*)prePlain)[1]=ntohl(z);
}

int Crypto::_rand()
{
    return rand();
}
