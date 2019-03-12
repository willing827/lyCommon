#ifndef __AES_H__
#define __AES_H__

#define BLK_SIZE	16
#define	TXT_SIZE	1024
#define	NUM_RND		11


#include <exception>
#include <cstring>
//#include "aesTable.h"

#define Uint8	unsigned char
#define Uint32	unsigned int

class Block
{
public:
	unsigned int *pBlkWord;
	unsigned char blkByte[BLK_SIZE];
};

class Aes
{

public:
	
	Aes(); // Constructor
	virtual ~Aes();	// Destructor

	int aesEncrypt(unsigned char *m_input_txt, 
					unsigned char *m_key, 
					unsigned char *pCipher);

	int aesDecrypt(unsigned char *pMesg, 
					unsigned char* pKey, 
					unsigned char* pResult,
					int msgLength);

    int aesEncryptStream(unsigned char *m_input_txt, 
        int input_Length,
        unsigned char *pKey, 
        unsigned char *pCipher);

	Block aesGetIV();
	
	void aesSetIV(unsigned char *pIV);

private:

	Block	mesg;
	Block	key;
	Block	initVec;
	Block	cipher;
	Block	rndKeys[NUM_RND];
	static unsigned char SBox[256];
	static unsigned char SBoxInv[256];

	void blockEnc(Block *pMesg, Block key);
	void blockDec(Block *pMesg, Block key);

	void byteSub(Block *pBlk);
	void invByteSub(Block *pBlk);

	void shiftRow(Block *pBlk);
	void invShiftRow(Block *pBlk);

	void mixColumn(Block *pBlk);
	void invMixColumn(Block *pBlk);

	void makeKeys(Block key);
	void getRndKeys(int rndNum, Block *pKey);


};



#endif
