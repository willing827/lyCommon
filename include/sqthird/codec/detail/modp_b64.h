/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
/* vi: set expandtab shiftwidth=4 tabstop=4: */

/**
 * \file
 * <PRE>
 * High performance base64 encoder / decoder
 * Version 1.3 -- 17-Mar-2006
 *
 * Copyright &copy; 2005, 2006, Nick Galbreath -- nickg [at] modp [dot] com
 * All rights reserved.
 *
 * http://modp.com/release/base64
 *
 * Released under bsd license.  See modp_b64.c for details.
 * </pre>
 *
 * The default implementation is the standard b64 encoding with padding.
 * It's easy to change this to use "URL safe" characters and to remove
 * padding.  See the modp_b64.c source code for details.
 *
 */

#ifndef MODP_B64
#define MODP_B64

#include <sqstd/sqinc.h>
namespace snqu {

/**
 * Encode a raw binary string into base 64.
 * src contains the bytes
 * len contains the number of bytes in the src
 * dest should be allocated by the caller to contain
 *   at least modp_b64_encode_len(len) bytes (see below)
 *   This will contain the null-terminated b64 encoded result
 * returns length of the destination string plus the ending null byte
 *    i.e.  the result will be equal to strlen(dest) + 1
 *
 * Example
 * 
 * \code
 * char* src = ...;
 * int srclen = ...; //the length of number of bytes in src
 * char* dest = (char*) malloc(modp_b64_encode_len);
 * int len = modp_b64_encode(dest, src, sourcelen);
 * if (len == -1) {
 *   printf("Error\n");
 * } else {
 *   printf("b64 = %s\n", dest);
 * }
 * \endcode
 *
 */
int modp_b64_encode(char* dest, const char* str, int len);

/**
 * Decode a base64 encoded string
 *
 * src should contain exactly len bytes of b64 characters.
 *     if src contains -any- non-base characters (such as white
 *     space, -1 is returned.
 *
 * dest should be allocated by the caller to contain at least
 *    len * 3 / 4 bytes.
 *
 * Returns the length (strlen) of the output, or -1 if unable to
 * decode
 *
 * \code
 * char* src = ...;
 * int srclen = ...; // or if you don't know use strlen(src)
 * char* dest = (char*) malloc(modp_b64_decode_len(srclen));
 * int len = modp_b64_decode(dest, src, sourcelen);
 * if (len == -1) { error }
 * \endcode
 */
int modp_b64_decode(char* dest, const char* src, int len);

/**
 * Given a source string of length len, this returns the amount of
 * memory the destination string should have.
 *
 * remember, this is integer math
 * 3 bytes turn into 4 chars
 * ceiling[len / 3] * 4 + 1
 *
 * +1 is for any extra null.
 */
#define modp_b64_encode_len(A) ((A+2)/3 * 4 + 1)

/**
 * Given a base64 string of length len,
 *   this returns the amount of memory required for output string
 *  It maybe be more than the actual number of bytes written.
 * NOTE: remember this is integer math
 * this allocates a bit more memory than traditional versions of b64
 * decode  4 chars turn into 3 bytes
 * floor[len * 3/4] + 2
 */
#define modp_b64_decode_len(A) (A / 4 * 3 + 2)

/**
 * Will return the strlen of the output from encoding.
 * This may be less than the required number of bytes allocated.
 *
 * This allows you to 'deserialized' a struct
 * \code
 * char* b64encoded = "...";
 * int len = strlen(b64encoded);
 *
 * struct datastuff foo;
 * if (modp_b64_encode_strlen(sizeof(struct datastuff)) != len) {
 *    // wrong size
 *    return false;
 * } else {
 *    // safe to do;
 *    if (modp_b64_decode((char*) &foo, b64encoded, len) == -1) {
 *      // bad characters
 *      return false;
 *    }
 * }
 * // foo is filled out now
 * \endcode
 */
#define modp_b64_encode_strlen(A) ((A + 2)/ 3 * 4)

#include <string>

inline void modp_replace_pad(std::string& s)
{
	char *p = &s[0];
	for (int i = 0; i < (int)s.size(); i++)
	{
		// '+','/' ==> '-', '*'
		if ('/' == p[i])
			p[i] = '*';
		else if ('+' == p[i])
			p[i] = '-';
	}
}

inline void cmodp_replace_pad(char *input, 
							  int input_size)
{
	char *p = &input[0];
	for (int i = 0; i < (int)input_size; i++)
	{
		// '+','/' ==> '-', '*'
		if ('/' == p[i])
			p[i] = '*';
		else if ('+' == p[i])
			p[i] = '-';
	}
}

inline void modp_restore_pad(std::string& s)
{
	char *p = &s[0];
	for (int i = 0; i < (int)s.size(); i++)
	{
		// '-', '*'==> '+','/' 
		if ('*' == p[i])
			p[i] = '/';
		else if ('-' == p[i])
			p[i] = '+';
	}
}

inline void cmodp_restore_pad(char *input, 
							  int input_size)
{
	char *p = &input[0];
	for (int i = 0; i < input_size; i++)
	{
		// '-', '*'==> '+','/' 
		if ('*' == p[i])
			p[i] = '/';
		else if ('-' == p[i])
			p[i] = '+';
	}
}

inline std::string& modp_b64_encode(std::string& s)
{
    std::string x(modp_b64_encode_len(s.size()), '\0');
    int d = modp_b64_encode(const_cast<char*>(x.data()), s.data(), s.size());
    x.erase(d, std::string::npos);
    s.swap(x);
    return s;
}

inline std::string modp_url_b64_encode(std::string& s)
{
	string x = modp_b64_encode(s);
	modp_replace_pad(x);
	return x;
}

inline int cmodp_url_b64_encode(const char *input, 
								int input_size, 
								char **output, 
								int *ouput_size)
{
	int pre_size = modp_b64_encode_len(input_size);
	char *b64_data = (char *)malloc(pre_size);
	int result_size = modp_b64_encode(b64_data, input, input_size);
	
	cmodp_replace_pad(b64_data, result_size);
	*output = b64_data;
	*ouput_size = result_size;
	return result_size;
}

/**
 * base 64 decode a string (self-modifing)
 * On failure, the string is empty.
 *
 * This function is for C++ only (duh)
 *
 * \param[in,out] s the string to be decoded
 * \return a reference to the input string
 */
inline std::string& modp_b64_decode(std::string& s)
{
    std::string x(modp_b64_decode_len(s.size()), '\0');
    int d = modp_b64_decode(const_cast<char*>(x.data()), s.data(), s.size());
    if (d < 0) {
        x.clear();
    } else {
        x.erase(d, std::string::npos);
    }
    s.swap(x);
    return s;
}

inline std::string modp_url_b64_decode(std::string& s)
{
	modp_restore_pad(s);
	return modp_b64_decode(s);
}

inline int cmodp_url_b64_decode(const char *input,
								int input_size,
								char **output,
								int *output_size)
{
	char *pre_buf;
	char *copy_input;
	int pre_size;

	pre_size = modp_b64_decode_len(input_size);
	copy_input = _strdup(input);
	pre_buf = (char *)malloc(pre_size);
	cmodp_restore_pad((char *)copy_input, input_size);
	int real_size = modp_b64_decode(pre_buf, copy_input, input_size);
	if (real_size > 0) {
		*output = pre_buf;
		*output_size = real_size;
	}
	else {
		free(pre_buf);
		*output_size = 0;
		*output = NULL;
	}

	free(copy_input);
	return real_size;
}

}
#endif /* MODP_B64 */
