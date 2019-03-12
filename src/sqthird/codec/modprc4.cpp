#include <codec/detail/modprc4.h>

namespace snqu {
	RC4Algo::RC4Algo() {};
	RC4Algo::~RC4Algo() {};

	void RC4Algo::rc4_encrypt_data(void * pData,int dwBufLen,char * szKey,int nKeyLen)
	{
		rc4_key key;

		prepare_key((unsigned char*)szKey,nKeyLen,&key);
		rc4((unsigned char*)pData,dwBufLen,&key);
	}

	void RC4Algo::prepare_key(unsigned char *key_data_ptr, int key_data_len, rc4_key *key)
	{
		unsigned char t;
		unsigned char index1;
		unsigned char index2;
		unsigned char* state;
		short counter;

		state = &key->state[0];
		for(counter = 0; counter < 256; counter++)
			state[counter] =(unsigned char)counter;
		key->x = 0;
		key->y = 0;
		index1 = 0;
		index2 = 0;
		for(counter = 0; counter < 256; counter++)
		{
			index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
			swap_byte(&state[counter], &state[index2]);
			index1 = (index1 + 1) % key_data_len;
		}
	}

	void RC4Algo::rc4(unsigned char *buffer_ptr, int buffer_len, rc4_key *key)
	{
		unsigned char t;
		unsigned char x;
		unsigned char y;
		unsigned char* state;
		unsigned char xorIndex;
		int counter;

		x = key->x;
		y = key->y;
		state = &key->state[0];
		for(counter = 0; counter < buffer_len; counter++)
		{
			x = (x + 1) % 256;
			y = (state[x] + y) % 256;
			swap_byte(&state[x], &state[y]);
			xorIndex = (state[x] + state[y]) % 256;
			buffer_ptr[counter] ^= state[xorIndex];
		}
		key->x = x;
		key->y = y;
	}
}