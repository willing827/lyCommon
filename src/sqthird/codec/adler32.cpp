/*****************************************************************************/
/* Includes:                                                                 */
/*****************************************************************************/
#include <string>
#include <codec/detail/adler32.h>
#define BASE 65521 

#pragma warning(disable: 4018)

#ifndef MULTIPLY_AS_A_FUNCTION
#define MULTIPLY_AS_A_FUNCTION 0
#endif

unsigned long adler32(const unsigned char *buf, int len)
{
    unsigned long adler = 1;
    unsigned long s1 = adler & 0xffff;
    unsigned long s2 = (adler >> 16) & 0xffff;

    int i;
    for (i = 0; i < len; i++) 
    {
        s1 = (s1 + buf[i]) % BASE;
        s2 = (s2 + s1) % BASE;
    }
    return (s2 << 16) + s1;
}