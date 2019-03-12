/***********************************************************
 * FILE NAME: aes.cpp with php 5.6
 *
 * DESCRIPTION: this file define various functions that are used to
 * encrypt or decrypt a message using Advance Standard Encryption.
 *
 * By: Khoa Vu
 * Date 2/22/03
 ************************************************************/

#include <iostream>
#include <cstring>
#include <exception>
#include <sqsafe/detail/phpAES.h>
#include "aesTable.h"
#include <stdlib.h>

//#define	DEBUG	1
#define cmin(a,b)    (((a) < (b)) ? (a) : (b))
/**************************
 * FUNCTION: Constructor
 *
 * DESCRIPTION: initial message block and key block and cipher block
 **************************/
Aes::Aes() 
{
	memset(&mesg, 0, sizeof(Block));
	memset(&key, 0, sizeof(Block));
	memset(&cipher, 0, sizeof(Block));
	memset(&rndKeys, 0, sizeof(Block)*NUM_RND);
}

/**************************
 * FUNCTION: Destructor
 *
 * DESCRIPTION: initial message block and key block and cipher block
 **************************/
Aes::~Aes()
{
	//delete [] key;
	//delete [] mesg;
	//delete [] cihper;

}

/**************************
 * FUNCTION: aesEncrypt(char *pMesg, char *pKey, char *pCipher)
 *
 * PARAMETERS: pMesg: pointer to message
 *				pKey: pointer to key
 *				pCihper: pointer to cipher text
 *
 * DESCRIPTION: This function encrypt the message using AES algorithm
 **************************/
int Aes::aesEncrypt(unsigned char *pMesg, 
					 unsigned char* pKey, 
					 unsigned char* pCipher)
{
	int i,j, msgLength, cpSize, len;
	Block	iv, tmp;
	
	len = 0;
	//memset(pCipher, 0, TXT_SIZE); // clear output entry

	memcpy(&key.blkByte, pKey, BLK_SIZE); // copy key to block of 16 bytes
	//pMesg += i;
	makeKeys(key);
	msgLength = strlen((const char *)pMesg);

    cpSize = cmin(msgLength, BLK_SIZE);
	// XOR the first message block with IV
	memcpy(&mesg.blkByte, pMesg, cpSize); // copy messeage to block of 16 bytes
	
	iv = aesGetIV();
	for(j = 0; j < BLK_SIZE; j++)
	{
		mesg.blkByte[j] ^= iv.blkByte[j];
	}

	blockEnc(&mesg, key);
	memcpy(pCipher, &mesg.blkByte, BLK_SIZE);
	pCipher += BLK_SIZE; len+=BLK_SIZE;
	pMesg += BLK_SIZE;

	for(i = BLK_SIZE; i < msgLength; i+= BLK_SIZE)
	{
		
		if ( strlen((const char *)pMesg) > BLK_SIZE)
			cpSize = BLK_SIZE;
		else
			cpSize = strlen((const char *)pMesg);

		memset(&tmp, 0, sizeof(Block)); // clear output entry
		memcpy(&tmp.blkByte, pMesg, cpSize); // copy messeage to block of 16 bytes
#if 1		
		for(j = 0; j < BLK_SIZE; j++)
		{
			//mesg.blkByte[j] ^= *pMesg;
			//pMesg++;
			mesg.blkByte[j] ^= tmp.blkByte[j];
		}
#endif
		blockEnc(&mesg, key);
		memcpy(pCipher, &mesg.blkByte, BLK_SIZE);
		pCipher += BLK_SIZE; len+=BLK_SIZE;
		
		pMesg += BLK_SIZE;

	}
	
	return len;
}

int Aes::aesEncryptStream(unsigned char *m_input_txt, 
                     int input_Length,
                     unsigned char *pKey, 
                     unsigned char *pCipher)
{
    int i,j, cpSize, len;
    Block	iv, tmp;

    len = 0;
    //memset(pCipher, 0, TXT_SIZE); // clear output entry

    memcpy(&key.blkByte, pKey, BLK_SIZE); // copy key to block of 16 bytes
    //pMesg += i;
    makeKeys(key);

	cpSize = cmin(input_Length, BLK_SIZE);
    // XOR the first message block with IV
    memcpy(&mesg.blkByte, m_input_txt, cpSize); // copy messeage to block of 16 bytes

    iv = aesGetIV();
    for(j = 0; j < BLK_SIZE; j++)
    {
        mesg.blkByte[j] ^= iv.blkByte[j];
    }

    blockEnc(&mesg, key);
    memcpy(pCipher, &mesg.blkByte, BLK_SIZE);
    pCipher += BLK_SIZE; len+=BLK_SIZE;
    m_input_txt += BLK_SIZE;

    for(i = BLK_SIZE; i < input_Length; i+= BLK_SIZE)
    {
        cpSize = input_Length - len;
        if (cpSize > BLK_SIZE)
            cpSize = BLK_SIZE;

        memset(&tmp, '\a', sizeof(Block)); // clear output entry
        memcpy(&tmp.blkByte, m_input_txt, cpSize); // copy messeage to block of 16 bytes
        
        for(j = 0; j < BLK_SIZE; j++)
        {
            mesg.blkByte[j] ^= tmp.blkByte[j];
        }
      
        blockEnc(&mesg, key);
        memcpy(pCipher, &mesg.blkByte, BLK_SIZE);
        pCipher += BLK_SIZE; len+=BLK_SIZE;

        m_input_txt += cpSize;
    }

    return len;
}

/**************************
 * FUNCTION: aesDecrypt(char *pMesg, char *pKey, char *pCipher)
 *
 * PARAMETERS: pMesg: pointer to message
 *				pKey: pointer to key
 *				pCihper: pointer to cipher text
 *
 * DESCRIPTION: This function encrypt the message using AES algorithm
 **************************/
int Aes::aesDecrypt(unsigned char *pMesg, 
					 unsigned char* pKey, 
					 unsigned char* pResult,
					 int msgLength )
{
	int i, j,len;
	unsigned char *pTmp;
	Block	iv;

	pTmp = pMesg;
	len=0;
//	memset(pCipher, 0, TXT_SIZE); // clear output entry

	memcpy(&key.blkByte, pKey, BLK_SIZE); // copy key to block of 16 bytes
	makeKeys(key);
//	msgLength = strlen((const char *)pMesg);
	
	
	// Decrypt the first message block
	memcpy(&mesg.blkByte, pMesg, BLK_SIZE); // copy messeage to block of 16 bytes
	blockDec(&mesg, key); 
	

	// XOR the first message block with IV 
	memset(&iv, 0, sizeof(Block));
	iv = aesGetIV();
	for(j = 0; j < BLK_SIZE; j++)
	{
		mesg.blkByte[j] ^= iv.blkByte[j];
	}
	
	memcpy(pResult, &mesg.blkByte, BLK_SIZE);
	pResult += BLK_SIZE;len+=BLK_SIZE;
	pMesg += BLK_SIZE;


	
	for(i = BLK_SIZE; i < msgLength; i+= BLK_SIZE)
	{

		memcpy(&mesg.blkByte, pMesg, BLK_SIZE); // copy messeage to block of 16 bytes
		blockDec(&mesg, key); 

		
#if 1		
		for(j = 0; j < BLK_SIZE; j++)
		{
			mesg.blkByte[j] ^= *pTmp;
			pTmp++;
		}
#endif
		//pTmp = pMesg;
		//pMesg = pTmp;
		memcpy(pResult, &mesg.blkByte, BLK_SIZE);
		pResult += BLK_SIZE;len+=BLK_SIZE;
		pMesg += BLK_SIZE;
	}
	
	return len;
}



// <<<< -----------------------------------
/**************************
 * FUNCTION: aesSetIV(unsigned char *pIV)
 *
 * PARAMETERS: 
 *				
 *
 * DESCRIPTION: set Initial vector
 **************************/
void Aes::aesSetIV(unsigned char *pIV)
{
	int	ivSize = 0;

	ivSize = strlen((const char *)pIV); //get IV size
	memset(&initVec, 0x00, sizeof(Block)); // clear Init Vector
	
	// Do not set initVec if its size is 0
	if (ivSize)
		memcpy(&initVec.blkByte, pIV, ivSize); // copy key to block of 16 bytes
}


/**************************
 * FUNCTION: aesGetIV(unsigned char *pIV)
 *
 * PARAMETERS: 
 *				
 *
 * DESCRIPTION: return Initial vector
 **************************/
Block Aes::aesGetIV()
{
	return initVec;
}


// ----------------------------------- >>>>

/**************************
 * FUNCTION: blockEnc
 *
 * PARAMETERS: 
 *
 * DESCRIPTION: 
 **************************/
void Aes::blockEnc(Block *pMesg, Block key)
{
//	Uint8 *pResult;
	Block	rndkey;
	int i,j;


	
	// add round key
	for(i = 0; i < BLK_SIZE; i++)
	{
		pMesg->blkByte[i] ^= key.blkByte[i];
	}

	// next nine rounds
	for(i = 1; i < 10; i++)
	{
		getRndKeys(i,&rndkey);
		byteSub(pMesg);
		shiftRow(pMesg);
		mixColumn(pMesg);
		
		// add round key
		for(j = 0; j < BLK_SIZE; j++)
		{
			pMesg->blkByte[j] ^= rndkey.blkByte[j];
		}
	}

	// final round
	getRndKeys(10,&rndkey);
	byteSub(pMesg);
	shiftRow(pMesg);
	
	// add round key
	for(i = 0; i < BLK_SIZE; i++)
	{
		pMesg->blkByte[i] ^= rndkey.blkByte[i];
	}


	//memcpy(pCipher, pResult, BLK_SIZE);
}




/**************************
 * FUNCTION: blockDec
 *
 * PARAMETERS: 
 *
 * DESCRIPTION: Decrypt block of 128 bits 
 **************************/
void Aes::blockDec(Block *pMesg, Block key)
{
//	Uint8 *pResult;
	Block	rndkey;
	int i,j;


	// check if we have a new key.  If so then 
	// make new key set else use old key
	
	// ------
	// ------
	// ------
	getRndKeys(10, &rndkey);

	// add round key
	for(i = 0; i < BLK_SIZE; i++)
	{
		pMesg->blkByte[i] ^= rndkey.blkByte[i];
	}
	invShiftRow(pMesg);
	invByteSub(pMesg);


	// next nine rounds
	for(i = 9; i > 0; i--)
	{
		getRndKeys(i,&rndkey);

		// add round key
		for(j = 0; j < BLK_SIZE; j++)
		{
			pMesg->blkByte[j] ^= rndkey.blkByte[j];
		}
		invMixColumn(pMesg);
		invShiftRow(pMesg);
		invByteSub(pMesg);
	}

	// final round
	getRndKeys(0,&rndkey);
	
	
	// add round key
	for(i = 0; i < BLK_SIZE; i++)
	{
		pMesg->blkByte[i] ^= rndkey.blkByte[i];
	}


	//memcpy(pCipher, pResult, BLK_SIZE);
}

/**************************
 * FUNCTION: byteSub
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/
void Aes::byteSub(Block *pBlk)
{
	int i;

	for(i = 0; i < BLK_SIZE; i++)
	{
		pBlk->blkByte[i] = SBox[pBlk->blkByte[i]];
	}
}

/**************************
 * FUNCTION: invByteSub
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/
void Aes::invByteSub(Block *pBlk)
{
	int i;

	for(i = 0; i < BLK_SIZE; i++)
	{
		pBlk->blkByte[i] = SBoxInv[pBlk->blkByte[i]];
	}
}

/**************************
 * FUNCTION: shiftRow
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/
void Aes::shiftRow(Block *pBlk)
{
	Block	tmp;

	tmp.blkByte[0] = pBlk->blkByte[0];
	tmp.blkByte[1] = pBlk->blkByte[5];
	tmp.blkByte[2] = pBlk->blkByte[10];
	tmp.blkByte[3] = pBlk->blkByte[15];

	tmp.blkByte[4] = pBlk->blkByte[4];
	tmp.blkByte[5] = pBlk->blkByte[9];
	tmp.blkByte[6] = pBlk->blkByte[14];
	tmp.blkByte[7] = pBlk->blkByte[3];

	tmp.blkByte[8] = pBlk->blkByte[8];
	tmp.blkByte[9] = pBlk->blkByte[13];
	tmp.blkByte[10] = pBlk->blkByte[2];
	tmp.blkByte[11] = pBlk->blkByte[7];

	tmp.blkByte[12] = pBlk->blkByte[12];
	tmp.blkByte[13] = pBlk->blkByte[1];
	tmp.blkByte[14] = pBlk->blkByte[6];
	tmp.blkByte[15] = pBlk->blkByte[11];

	memcpy(pBlk, &tmp, sizeof(Block));
}

/**************************
 * FUNCTION: invShiftRow
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/
void Aes::invShiftRow(Block *pBlk)
{
	Block	tmp;

	tmp.blkByte[0] = pBlk->blkByte[0];
	tmp.blkByte[1] = pBlk->blkByte[13];
	tmp.blkByte[2] = pBlk->blkByte[10];
	tmp.blkByte[3] = pBlk->blkByte[7];

	tmp.blkByte[4] = pBlk->blkByte[4];
	tmp.blkByte[5] = pBlk->blkByte[1];
	tmp.blkByte[6] = pBlk->blkByte[14];
	tmp.blkByte[7] = pBlk->blkByte[11];

	tmp.blkByte[8] = pBlk->blkByte[8];
	tmp.blkByte[9] = pBlk->blkByte[5];
	tmp.blkByte[10] = pBlk->blkByte[2];
	tmp.blkByte[11] = pBlk->blkByte[15];

	tmp.blkByte[12] = pBlk->blkByte[12];
	tmp.blkByte[13] = pBlk->blkByte[9];
	tmp.blkByte[14] = pBlk->blkByte[6];
	tmp.blkByte[15] = pBlk->blkByte[3];

	memcpy(pBlk, &tmp, sizeof(Block));
}


/**************************
 * FUNCTION: mixColumn
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/
void Aes::mixColumn(Block *pBlk)
{
	Block	tmp;
	int		i;

	for(i = 0; i < 4; i++)
	{
		tmp.blkByte[i*4] = f2(pBlk->blkByte[4*i]) ^ f3(pBlk->blkByte[4*i+1]) ^
						pBlk->blkByte[4*i+2] ^ pBlk->blkByte[4*i+3];

		tmp.blkByte[i*4+1] = pBlk->blkByte[4*i] ^ f2(pBlk->blkByte[4*i+1]) ^
						f3(pBlk->blkByte[4*i+2]) ^ pBlk->blkByte[4*i+3];

		tmp.blkByte[i*4+2] = pBlk->blkByte[4*i] ^ pBlk->blkByte[4*i+1] ^
						f2(pBlk->blkByte[4*i+2]) ^ f3(pBlk->blkByte[4*i+3]);

		tmp.blkByte[i*4+3] = f3(pBlk->blkByte[4*i]) ^ pBlk->blkByte[4*i+1] ^
						 pBlk->blkByte[4*i+2] ^ f2(pBlk->blkByte[4*i+3]);
	}
	memcpy(pBlk, &tmp, sizeof(Block));

}


/**************************
 * FUNCTION: invMixColumn
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/
void Aes::invMixColumn(Block *pBlk)
{
	Block	tmp;
	int		i;

	for(i = 0; i < 4; i++)
	{
		tmp.blkByte[4*i] = fe(pBlk->blkByte[4*i]) ^ fb(pBlk->blkByte[4*i+1]) ^
						fd(pBlk->blkByte[4*i+2]) ^ f9(pBlk->blkByte[4*i+3]);

		tmp.blkByte[4*i+1] = f9(pBlk->blkByte[4*i]) ^ fe(pBlk->blkByte[4*i+1]) ^
						fb(pBlk->blkByte[4*i+2]) ^ fd(pBlk->blkByte[4*i+3]);

		tmp.blkByte[4*i+2] = fd(pBlk->blkByte[4*i]) ^ f9(pBlk->blkByte[4*i+1]) ^
						fe(pBlk->blkByte[4*i+2]) ^ fb(pBlk->blkByte[4*i+3]);

		tmp.blkByte[4*i+3] = fb(pBlk->blkByte[4*i]) ^ fd(pBlk->blkByte[4*i+1]) ^
						 f9(pBlk->blkByte[4*i+2]) ^ fe(pBlk->blkByte[4*i+3]);
	}
	memcpy(pBlk, &tmp, sizeof(Block));

}



/**************************
 * FUNCTION: makeKeys
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/
void Aes::makeKeys(Block key)
{
	int i, j, k;
	Uint32 tmp, wi_4, wi_1;
	Uint8	*pTmp;
	Uint32	rndCnst[11] = {0x00, 0x01, 0x02, 
							0x04, 0x08, 0x10,
							0x20, 0x40, 0x80, 
							0x1b, 0x36};

	memcpy(&rndKeys[0], &key, sizeof(Block)); // copy the master key to block 0
	//rndKeys[0].pBlkWord = (Uint32 *)&rndKeys[0].blkByte + 3;

	for(i = 1; i < NUM_RND; i++)
	{
		// do transformation 
		rndKeys[i-1].pBlkWord = (Uint32 *)&rndKeys[i-1].blkByte + 3;
		rndKeys[i].pBlkWord = (Uint32 *)&rndKeys[i].blkByte; // set address
		
		//shift bytes
		tmp = (*rndKeys[i-1].pBlkWord << 24) |
				((*rndKeys[i-1].pBlkWord & 0xffffff00) >> 8);
		
		tmp = tmp;
		pTmp = (Uint8 *)&tmp;

		// SBox substitution
		for(k = 0; k<4; k++)
		{
			*pTmp = SBox[*pTmp];
			pTmp++;
		}
		
		// result
		rndKeys[i-1].pBlkWord = (Uint32 *)&rndKeys[i-1].blkByte;
		*rndKeys[i].pBlkWord = tmp ^ rndCnst[i] ^ *rndKeys[i-1].pBlkWord;  


		rndKeys[i].pBlkWord ++;
		rndKeys[i-1].pBlkWord ++;

		for(j = 1; j<4; j++)
		{
			wi_4 = *(rndKeys[i].pBlkWord - 1);
			wi_1 = *rndKeys[i-1].pBlkWord;
			*rndKeys[i].pBlkWord = wi_1 ^ wi_4;
			rndKeys[i].pBlkWord += 1;
			rndKeys[i-1].pBlkWord ++;
			
		}
		rndKeys[i].pBlkWord = (Uint32 *)&rndKeys[i].blkByte; // reset address
		rndKeys[i-1].pBlkWord = (Uint32 *)&rndKeys[i-1].blkByte;
	}
}


/**************************
 * FUNCTION: getRndKeys
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/
void Aes::getRndKeys(int rndNum, Block *pKey)
{
	memcpy(pKey, &rndKeys[rndNum], sizeof(Block));
}















//-----------------------------------------------

#if 0
/**************************
 * FUNCTION: shiftRow
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/
void Aes::shiftRow(Block *pBlk)
{
	//Uint32	tmp;

	pBlk->pBlkWord = (unsigned int *)pBlk->blkByte;

	pBlk->pBlkWord ++; // shift second row
	*pBlk->pBlkWord = (*pBlk->pBlkWord << 24) | 
		((*pBlk->pBlkWord & 0xffffff00) >> 8); // shift second row

	pBlk->pBlkWord ++; // shift third row
	*pBlk->pBlkWord = (*pBlk->pBlkWord << 16) | 
		((*pBlk->pBlkWord & 0xffff0000) >> 16); // shift third row

	pBlk->pBlkWord ++; // shift fourth row
	*pBlk->pBlkWord = (*pBlk->pBlkWord << 8) | 
		((*pBlk->pBlkWord & 0xffffff00) >> 24); // shift fourth row
}

/**************************
 * FUNCTION: invShiftRow
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/
void Aes::invShiftRow(Block *pBlk)
{
	//Uint32	tmp;

	pBlk->pBlkWord = (unsigned int *)pBlk->blkByte;

	pBlk->pBlkWord ++; // shift second row
	*pBlk->pBlkWord = (*pBlk->pBlkWord << 8) | 
		((*pBlk->pBlkWord & 0xffffff00) >> 24); // shift second row

	pBlk->pBlkWord ++; // shift third row
	*pBlk->pBlkWord = (*pBlk->pBlkWord << 16) | 
		((*pBlk->pBlkWord & 0xffff0000) >> 16); // shift third row

	pBlk->pBlkWord ++; // shift fourth row
	*pBlk->pBlkWord = (*pBlk->pBlkWord << 24) | 
		((*pBlk->pBlkWord & 0xffffff00) >> 8); // shift fourth row
}



/**************************
 * FUNCTION: mixColumn
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/
void Aes::mixColumn(Block *pBlk)
{
	Block	tmp;
	int		i;

	for(i = 0; i < 4; i++)
	{
		tmp.blkByte[i] = f2(pBlk->blkByte[i]) ^ f3(pBlk->blkByte[4+i]) ^
						pBlk->blkByte[8+i] ^ pBlk->blkByte[12+i];

		tmp.blkByte[4+i] = pBlk->blkByte[i] ^ f2(pBlk->blkByte[4+i]) ^
						f3(pBlk->blkByte[8+i]) ^ pBlk->blkByte[12+i];

		tmp.blkByte[8+i] = pBlk->blkByte[i] ^ pBlk->blkByte[4+i] ^
						f2(pBlk->blkByte[8+i]) ^ f3(pBlk->blkByte[12+i]);

		tmp.blkByte[12+i] = f3(pBlk->blkByte[i]) ^ pBlk->blkByte[4+i] ^
						 pBlk->blkByte[8+i] ^ f2(pBlk->blkByte[12+i]);
	}
	memcpy(pBlk, &tmp, sizeof(Block));

}


/**************************
 * FUNCTION: invMixColumn
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/
void Aes::invMixColumn(Block *pBlk)
{
	Block	tmp;
	int		i;

	for(i = 0; i < 4; i++)
	{
		tmp.blkByte[i] = fe(pBlk->blkByte[i]) ^ fb(pBlk->blkByte[4+i]) ^
						fd(pBlk->blkByte[8+i]) ^ f9(pBlk->blkByte[12+i]);

		tmp.blkByte[4+i] = f9(pBlk->blkByte[i]) ^ fe(pBlk->blkByte[4+i]) ^
						fb(pBlk->blkByte[8+i]) ^ fd(pBlk->blkByte[12+i]);

		tmp.blkByte[8+i] = fd(pBlk->blkByte[i]) ^ f9(pBlk->blkByte[4+i]) ^
						fe(pBlk->blkByte[8+i]) ^ fb(pBlk->blkByte[12+i]);

		tmp.blkByte[12+i] = fb(pBlk->blkByte[i]) ^ fd(pBlk->blkByte[4+i]) ^
						 f9(pBlk->blkByte[8+i]) ^ fe(pBlk->blkByte[12+i]);
	}
	memcpy(pBlk, &tmp, sizeof(Block));

}
#endif

/**************************
 * FUNCTION: 
 *
 * PARAMETERS: 
 *
 * DESCRIPTION:
 **************************/