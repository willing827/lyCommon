/*------------------------------------------------------------------------- */
/*  minialgo.h																 */
/*                                                                           */
/*  History                                                                  */
/*      12/05/2015                                                           */
/*                                                                           */
/*  Author                                                                   */
/*      Guo Lei																 */
/*                                                                           */
/*  Abstract																 */	
/*		一些小算法															 */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*--------------------------------------------------------------------------*/
#ifndef MINIALGO_H
#define MINIALGO_H

#include <sqstd/sqinc.h>
namespace snqu { namespace safe {
	void simple_swap_byte_encrypt(uint8* out, uint8* in, int len);
	void internal_simple_encrypt_data(string& data, const string& key);
}}
#endif //MINIALGO_H