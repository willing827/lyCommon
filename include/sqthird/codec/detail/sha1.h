#ifndef _SHA1_H_ 
#define _SHA1_H_ 
 
#include "stdint.h"
#include <sqstd/sqinc.h>
 
namespace snqu {
#ifndef _SHA_enum_ 
#define _SHA_enum_ 
enum 
{ 
    shaSuccess = 0, 
    shaNull,            /* ��ָʾ���� */ 
    shaInputTooLong,    /* ��������̫����ʾ */ 
    shaStateError       /* called Input after Result --������������֮ */ 
}; 
#endif 
#define SHA1HashSize 20 
 
/* 
 *  �������ֽṹ���������������Ϣ for the SHA-1 
 *  hashing operation 
 */ 
typedef struct SHA1Context 
{ 
    uint32_t Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */ 
 
    uint32_t Length_Low;            /* Message length in bits      */ 
    uint32_t Length_High;           /* Message length in bits      */ 
 
                               /* Index into message block array   */ 
    int_least16_t Message_Block_Index; 
    uint8_t Message_Block[64];      /* 512-bit message blocks      */ 
 
    int Computed;               /* Is the digest computed?         */ 
    int Corrupted;             /* Is the message digest corrupted? */ 
} SHA1Context; 
 
/* 
 *  ����ԭ�� 
 */ 
int SHA1Reset(  SHA1Context *); 
int SHA1Input(  SHA1Context *, 
                const uint8_t *, 
                unsigned int); 
int SHA1Result( SHA1Context *, 
                uint8_t Message_Digest[SHA1HashSize]); 

string SHA1Algo_Variation(const string& dataInput);
string SHA1Algo_Base(const string& dataInput, int repeatCount);
}
#endif /*_SHA1_H_*/ 