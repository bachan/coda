/*
 * Copyright (C) 2007, libturglem development team.
 *
 * This file is released under the LGPL.
*/

#ifndef __NTYBNTBRGVRF_WORLDLANGUAGE_UTF8_CHARSET_ADAPTER_H__
#define __NTYBNTBRGVRF_WORLDLANGUAGE_UTF8_CHARSET_ADAPTER_H__

#include <MAFSA/adapter.h>

/*

	0 1 2 3 4 5 6 7 8 9
	_ -
	ABCDEFGHIJKLMNOPQRSTUVWXYZ
	АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ

*/

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LETTER_WLA 70

ssize_t conv_s2l_wla_utf8(const char *s, MAFSA_letter *l, size_t sz);
size_t conv_l2s_wla_utf8(const MAFSA_letter *l, size_t sz_l, char *s, size_t sz_s);
	
#ifdef __cplusplus
}
#endif

#endif /* __NTYBNTBRGVRF_WORLDLANGUAGE_UTF8_CHARSET_ADAPTER_H__ */

