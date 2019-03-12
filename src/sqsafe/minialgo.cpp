#include "stdafx.h"
#include "minialgo.h"
#include <vmp/sqvmsdk.h>

namespace snqu { namespace safe {
void simple_swap_byte_encrypt(uint8* out, uint8* in, int len)
{
	BYTE  byascii_src = 0;
	BYTE  hbyte = 0, lbyte = 0;
	BYTE  xorkey = 0x01;
	int i = 0;

	if (len <= 0)
		return;

	BYTE *tmpbuf = (BYTE *)LocalAlloc(LPTR, len);
	if (!tmpbuf) {
		return;
	}

	_VMProtectBegin(__FUNCTION__);
	// ½»»»ÆæÅ¼×Ö½ÚË³Ðò
	while (i < len) {
		if ((i+1) < len) {
			tmpbuf[i] = in[i+1];
			tmpbuf[i+1] = in[i];
		}
		else {
			tmpbuf[i] = in[i];
		}

		i += 2;
	}

	// ½»»»¸ßµÍÎ»
	for (i = 0; i < len; i++) {
		byascii_src = (BYTE)tmpbuf[i];

		// 11110000
		hbyte = (byascii_src & 0xf0) >> 4;

		// 00001111
		lbyte = (byascii_src & 0x0f) << 4;
		byascii_src = lbyte | hbyte;

		if (0x00 == xorkey) xorkey++;
		out[i] = (char)(byascii_src ^ (xorkey++));
	}

	LocalFree((HLOCAL)tmpbuf);
	_VMProtectEnd();
}


void map_encrypt_key(uint8 *decode_buff, uint8 *key_buff, int key_size)
{
	uint8 KeyMap[0x100] = {0}; 
	int  i = 0;
	uint8 cbTmp = 0;
	uint8 cbIdx = 0;

	_VMProtectBegin(__FUNCTION__);
	for (i = 0;i < 0x100; i++) {  
		decode_buff[i] = i;
		KeyMap[i] = key_buff[i % key_size];            
	}


	for (i = 0; i < 0x100; i++) {
		cbIdx = decode_buff[i] + cbIdx + KeyMap[i];
		cbTmp = decode_buff[i];
		decode_buff[i] = decode_buff[cbIdx];
		decode_buff[cbIdx] = cbTmp;
	}
	_VMProtectEnd();
}

void decode_buffer(uint8 *decode_buff, uint8 *key_buff,
				   int key_size, uint8 *new_buff)
{
	uint8    cnIdx = 0; 
	uint8    n = 0; 
	uint8    m = 0; 
	uint8    nTmp = 0;
	uint8    i  = 0;

	_VMProtectBegin(__FUNCTION__);
	for (i = 0 ; i< key_size; i++) {
		cnIdx = i + 1;   
		n = decode_buff[cnIdx];
		m += n;

		decode_buff[cnIdx] = decode_buff[m];
		decode_buff[m] = n;
		nTmp = n + decode_buff[cnIdx];
		new_buff[i] = decode_buff[nTmp] ^ key_buff[i];
	}

	RtlZeroMemory(decode_buff, 0x100);
	_VMProtectEnd();
}

void internal_simple_encrypt_data(string& data, const string& key)
{
	_VMProtectBegin(__FUNCTION__);

	int data_size = data.size();
	int key_size = key.size();
	uint8  key_buff[0x100] = {0};
	string t(data_size, '\0');
	
	if (data_size > 0) {
		if (key_size > 0x100) 
			key_size = 0x100;

		map_encrypt_key(key_buff, (uint8*)&key[0], key_size);
		decode_buffer(key_buff, (uint8*)&data[0], data_size, (uint8*)&t[0]);
		memcpy((uint8*)&data[0], t.c_str(), data_size);
	}

	_VMProtectEnd();
}

}}