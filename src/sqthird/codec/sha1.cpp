#include <codec/detail/sha1.h>
#include <vmp/sqvmsdk.h>

namespace snqu {
/*  
 *  ������Ϊ SHA1 ��������λ�� ֮����  
 */
#define SHA1CircularShift(bits,word) (((word) << (bits)) | ((word) >> (32-(bits))))   
   
   
/* �ֲ�����ԭ�� */   
void SHA1PadMessage(SHA1Context *);    /*  ���������Ϣָ��  */   
void SHA1ProcessMessageBlock(SHA1Context *);   
   
/*  
 *  SHA1Reset  
 *    
 *  ����Ϊ���ݳ�ʼ��֮����  
 *  Parameters:���������ã�  
 *  context: [in/out]  
 *  The context to reset.  
 *  
 */   
int SHA1Reset(SHA1Context *context)   
{   
    if (!context)   
    {   
        return shaNull;   
    }   
   
    context->Length_Low             = 0;   
    context->Length_High            = 0;   
    context->Message_Block_Index    = 0;   
   
    context->Intermediate_Hash[0]   = 0x67452301;   
    context->Intermediate_Hash[1]   = 0xEFCDAB89;   
    context->Intermediate_Hash[2]   = 0x98BADCFE;   
    context->Intermediate_Hash[3]   = 0x10325476;   
    context->Intermediate_Hash[4]   = 0xC3D2E1F0;   
   
    context->Computed   = 0;   
    context->Corrupted  = 0;   
    return shaSuccess;   
}   
   
/*  
 *  SHA1Result  
 *  
 *  ����Ϊsha-1���������  
 *:  
 *  ���㷨���᷵��һ��160���ص���ϢժҪ����  
 *  
 *  ��������������  
 *  
 */   
int SHA1Result( SHA1Context *context,   
                uint8_t Message_Digest[SHA1HashSize])   
{   
    int i;   
   
    if (!context || !Message_Digest)   
    {   
        return shaNull;   
    }   
   
    if (context->Corrupted)   
    {   
        return context->Corrupted;   
    }   
   
    if (!context->Computed)   
    {   
        SHA1PadMessage(context);   
        for(i=0; i<64; ++i)   
        {   
            /* ��Ϣ���� */   
            context->Message_Block[i] = 0;   
        }   
        context->Length_Low = 0;    /* �������� */   
        context->Length_High = 0;   
        context->Computed = 1;   
    }   
   
    for(i = 0; i < SHA1HashSize; ++i)   
    {   
        Message_Digest[i] = context->Intermediate_Hash[i>>2]   
                            >> 8 * ( 3 - ( i & 0x03 ) );   
    }   
   
    return shaSuccess;   
}   
   
/*  
 *  ����Ϊsha-1����������  
 *  
 *  ���յ�λ����Ϊ8�ֽڱ�������Ϣ  
 *  
 */   
int SHA1Input(    SHA1Context    *context,   
                  const uint8_t  *message_array,   
                  unsigned       length)   
{   
    if (!length)   
    {   
        return shaSuccess;   
    }   
   
    if (!context || !message_array)   
    {   
        return shaNull;   
    }   
   
    if (context->Computed)   
    {   
        context->Corrupted = shaStateError;   
        return shaStateError;   
    }   
   
    if (context->Corrupted)   
    {   
         return context->Corrupted;   
    }   
    while(length-- && !context->Corrupted)   
    {   
    context->Message_Block[context->Message_Block_Index++] =   
                    (*message_array & 0xFF);   
   
    context->Length_Low += 8;   
    if (context->Length_Low == 0)   
    {   
        context->Length_High++;   
        if (context->Length_High == 0)   
        {   
            /* Message is too long */   
            context->Corrupted = 1;   
        }   
    }   
   
    if (context->Message_Block_Index == 64)   
    {   
        SHA1ProcessMessageBlock(context);   
    }   
   
    message_array++;   
    }   
   
    return shaSuccess;   
}   
   
/*  
 *  ����Ϊsha-1��Ϣ��������  
 *    
 *  ��Ϣ�鳤��Ϊ�̶�֮512����  
 *  
 */   
void SHA1ProcessMessageBlock(SHA1Context *context)   
{   
    const uint32_t K[] =    {       /* Constants defined in SHA-1   */   
                            0x5A827999,   
                            0x6ED9EBA1,   
                            0x8F1BBCDC,   
                            0xCA62C1D6   
                            };   
    int           t;                 /* ѭ������ */   
    uint32_t      temp;              /* ��ʱ���� */   
    uint32_t      W[80];             /* ��˳��   */   
    uint32_t      A, B, C, D, E;     /* ����ϵͳ���̻���� */   
   
    /*  
     *  ����Ϊ��ʼ����W�����е�ͷ16������ 
     */   
    for(t = 0; t < 16; t++)   
    {   
        W[t] = context->Message_Block[t * 4] << 24;   
        W[t] |= context->Message_Block[t * 4 + 1] << 16;   
        W[t] |= context->Message_Block[t * 4 + 2] << 8;   
        W[t] |= context->Message_Block[t * 4 + 3];   
    }   
   
    for(t = 16; t < 80; t++)   
    {   
       W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);   
    }   
   
    A = context->Intermediate_Hash[0];   
    B = context->Intermediate_Hash[1];   
    C = context->Intermediate_Hash[2];   
    D = context->Intermediate_Hash[3];   
    E = context->Intermediate_Hash[4];   
   
    /*    
     *  ����Ϊ�����㷨����֮��ѧ������������㷨����
     */   
   
    for(t = 0; t < 20; t++)   
    {   
        temp =  SHA1CircularShift(5,A) +   
                ((B & C) | ((~B) & D)) + E + W[t] + K[0];   
        E = D;   
        D = C;   
        C = SHA1CircularShift(30,B);   
   B = A;   
        A = temp;   
    }   
   
    for(t = 20; t < 40; t++)   
    {   
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];   
        E = D;   
        D = C;   
        C = SHA1CircularShift(30,B);   
        B = A;   
        A = temp;   
    }   
   
    for(t = 40; t < 60; t++)   
    {   
        temp = SHA1CircularShift(5,A) +   
               ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];   
        E = D;   
        D = C;   
        C = SHA1CircularShift(30,B);   
        B = A;   
        A = temp;   
    }   
   
    for(t = 60; t < 80; t++)   
    {   
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];   
        E = D;   
        D = C;   
        C = SHA1CircularShift(30,B);   
        B = A;   
        A = temp;   
    }   
   
     
    /*    
     *  ����Ϊ�����㷨��80�������һ��������  
     */   
    context->Intermediate_Hash[0] += A;   
    context->Intermediate_Hash[1] += B;   
    context->Intermediate_Hash[2] += C;   
    context->Intermediate_Hash[3] += D;   
    context->Intermediate_Hash[4] += E;   
   
    context->Message_Block_Index = 0;   
}   
   
   
/*  
 *  SHA1PadMessage  
 *  �������ģ��  
 */   
   
void SHA1PadMessage(SHA1Context *context)   
{   
   
    if (context->Message_Block_Index > 55)   
    {   
        context->Message_Block[context->Message_Block_Index++] = 0x80;   
        while(context->Message_Block_Index < 64)   
        {   
            context->Message_Block[context->Message_Block_Index++] = 0;   
        }   
   
        SHA1ProcessMessageBlock(context);   
   
        while(context->Message_Block_Index < 56)   
        {   
            context->Message_Block[context->Message_Block_Index++] = 0;   
        }   
    }   
    else   
    {   
        context->Message_Block[context->Message_Block_Index++] = 0x80;   
        while(context->Message_Block_Index < 56)   
        {   
            context->Message_Block[context->Message_Block_Index++] = 0;   
        }   
    }   
   
    /*  
     *  �����64λ����Ϊ���ݳ���  
     */   
    context->Message_Block[56] = context->Length_High >> 24;   
    context->Message_Block[57] = context->Length_High >> 16;   
    context->Message_Block[58] = context->Length_High >> 8;   
    context->Message_Block[59] = context->Length_High;   
    context->Message_Block[60] = context->Length_Low >> 24;   
    context->Message_Block[61] = context->Length_Low >> 16;   
    context->Message_Block[62] = context->Length_Low >> 8;   
    context->Message_Block[63] = context->Length_Low;   
   
    SHA1ProcessMessageBlock(context);   
}

string bytesToHexString(const uint8_t *input, size_t length) 
{
	const char HEX[16] = {
		'0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'a', 'b',
		'c', 'd', 'e', 'f'
	};

	string str;
	str.reserve(length << 1);
	for(size_t i = 0; i < length; i++) {
		int t = input[i];
		int a = t / 16;
		int b = t % 16;
		str.append(1, HEX[a]);
		str.append(1, HEX[b]);
	}
	return str;
}

string SHA1Algo_Base(const string& dataInput, int repeatCount)
{
	SHA1Context sha;
	uint8_t Message_Digest[20];
	string result("");

	_VMProtectBegin(__FUNCTION__);
	int err = SHA1Reset(&sha);
	if (!err)
	{
		for (int i = 0; i < repeatCount; i++)
		{
			err = SHA1Input(&sha, (const unsigned char *)dataInput.c_str(), dataInput.size());   
			if (err)   
				break;    /* out of for i loop */ 
		}

		err = SHA1Result(&sha, Message_Digest);
		if (err)   
		{
			/* SHA1Result error */
		}
		else
		{
			result = bytesToHexString(Message_Digest, sizeof(Message_Digest));
		}
	}

	_VMProtectEnd();
	return result;
}

string SHA1Algo_Variation(const string& dataInput)
{
	string result("");

	_VMProtectBegin(__FUNCTION__);
	auto base1Sha1 = SHA1Algo_Base(dataInput, 60);
	if (!base1Sha1.empty())
	{
		auto xdata = base1Sha1.substr(3, 16);
		result = SHA1Algo_Base(xdata, 40);
	}
	_VMProtectEnd();

	return result;
}
}