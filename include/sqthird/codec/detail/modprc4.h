#ifndef __SQRC4_H__
#define __SQRC4_H__

#include <sqstd/sqinc.h>

typedef struct _tag_rc4_key
{
	unsigned char state[256];
	unsigned char x;
	unsigned char y;
} rc4_key;

#define swap_byte(x,y) t = *(x); *(x) = *(y); *(y) = t

namespace snqu {
	class RC4Algo
	{
	public:
		RC4Algo();
		virtual ~RC4Algo();

	public:
		static void rc4_encrypt_data(void * pData,int dwBufLen,char *szKey,int nKeyLen);
		static void prepare_key(unsigned char *key_data_ptr, int key_data_len, rc4_key *key);
		static void rc4(unsigned char *buffer_ptr, int buffer_len, rc4_key *key);
	};
}
#endif //__SQRC4_H__