/*
 * Copyright (C) 2007, libturglem development team.
 *
 * This file is released under the LGPL.
*/

#ifndef __EGRTGERTGERTV_SIMPLE_LATIN_ADAPTER_H__
#define __EGRTGERTGERTV_SIMPLE_LATIN_ADAPTER_H__

#include <MAFSA/adapter.h>

/*

	0 1 2 3 4 5 6 7 8 9
	_ -
	ABCDEFGHIJKLMNOPQRSTUVWXYZ

*/

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LETTER_SLA 40

ssize_t conv_s2l_sla(const char *s, MAFSA_letter *l, size_t sz);
size_t conv_l2s_sla(const MAFSA_letter *l, size_t sz_l, char *s, size_t sz_s);

ssize_t conv_b2l_sla(const char *s, size_t sz_s, MAFSA_letter *l, size_t sz_l);
size_t conv_l2b_sla(const MAFSA_letter *l, size_t sz_l, char *s, size_t sz_s);

#ifdef __cplusplus
}
#endif

#endif /* __EGRTGERTGERTV_SIMPLE_LATIN_ADAPTER_H__ */

