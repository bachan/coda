#include "uac.h"
#include "sla.h"
#include <MAFSA/automaton.h>
#include <stdio.h>

static MAFSA_automaton uac_ma;

int uac_init(const char *fname)
{
	uac_ma = MAFSA_automaton_load_from_binary_file(fname, 0);
	if (!uac_ma) return -1;
	return 0;
}

void uac_free()
{
	MAFSA_automaton_close(uac_ma);
}

void MAFSACALL uac_determine_callback(void *user_data, const MAFSA_letter *l, size_t sz)
{
/*	char x[1024]; */
	unsigned int _t;
	uint64_t _v;
	uint64_t *V = (uint64_t*)user_data;

	if (sz < 4) return;
	_t = l[sz - 3];
	_v = (l[sz - 2] - 12) * 26 + l[sz - 1] - 12;

	switch (_t)
	{
		case 1:
			if ((*V & 0xFF) == 0)
			{
				*V += _v;
			}
			break;
		case 2:
			if ((*V & 0xFF00) == 0)
			{
				*V += (_v << 8);
			}
			break;
		case 3:
			*V |= (_v << 16);
			break;
		case 4:
			*V |= (_v << 24);
			break;
		case 5:
			*V |= (_v << 32);
			break;
	}

/*
	conv_l2s_sla(l, sz, x, 1024);
	printf("%s : %d: %d\n", x, _t, _v);
*/
}

uint32_t uac_determine(const char *s)
{
	uint64_t ret = 0;

	MAFSA_letter tmp[1024];
	MAFSA_letter l[1024];
	ssize_t ssz;

	int has_windows_ce = ((ret & ((uint64_t)1 << 33)) != 0);
	int has_windows = ((ret & ((uint64_t)1 << 35)) != 0);
	int has_msie = ((ret & ((uint64_t)1 << 34)) != 0);

	ssz = conv_s2l_sla(s, l, 1024);
	MAFSA_automaton_search_enumerate(uac_ma, l, ssz, tmp, 1024, &ret, MAX_LETTER_SLA, uac_determine_callback);

	/* Linux bit */
	if (((ret & 0xFF) == 0) && ((ret & ((uint64_t)1 << 32)) != 0)) ret |= 80;

	/* msie bit */
	if (((ret & 0xFF00) == 0) && ((ret & ((uint64_t)1 << 34)) != 0)) ret |= 16 << 8;

	/* no browser 0x1F (IE) if Opera exists */
	if (((ret & 0xF000) == 0x1000) && ((ret & ((uint64_t)1 << 38)) != 0)) ret = (ret & 0xFFFF00FF) + 0x8000;

	/* opera bit */
	if (((ret & 0xFF00) == 0) && ((ret & ((uint64_t)1 << 38)) != 0)) ret |= (128 << 8);

	/* no Safari if Chrome exists */
	if ((ret & ((uint64_t)1 << 39)) != 0) ret = (ret & 0xFFFF00FF) + 0xA300;

	/* windows ce bit */
	if (((ret & 0xFF) == 0) && has_windows_ce) ret |= 128;

	/* windows bit */
	if (((ret & 0xFF) == 0) && has_windows) ret |= 32;

	/* msie bit */
	if (((ret & 0xFF) == 0) && has_msie) ret |= 32;

	return (ret & 0xFFFFFFFF);
}

uint32_t uac_determine_binary(const char *s, size_t sz)
{
	uint64_t ret = 0;

	MAFSA_letter tmp[1024];
	MAFSA_letter l[1024];
	ssize_t ssz;

	int has_windows_ce = ((ret & ((uint64_t)1 << 33)) != 0);
	int has_windows = ((ret & ((uint64_t)1 << 35)) != 0);
	int has_msie = ((ret & ((uint64_t)1 << 34)) != 0);

	ssz = conv_b2l_sla(s, sz, l, 1024);
	MAFSA_automaton_search_enumerate(uac_ma, l, ssz, tmp, 1024, &ret, MAX_LETTER_SLA, uac_determine_callback);

	/* Linux bit */
	if (((ret & 0xFF) == 0) && ((ret & ((uint64_t)1 << 32)) != 0)) ret |= 80;

	/* msie bit */
	if (((ret & 0xFF00) == 0) && ((ret & ((uint64_t)1 << 34)) != 0)) ret |= 16 << 8;

	/* no browser 0x1F (IE) if Opera exists */
	if (((ret & 0xF000) == 0x1000) && ((ret & ((uint64_t)1 << 38)) != 0)) ret = (ret & 0xFFFF00FF) + 0x8000;

	/* opera bit */
	if (((ret & 0xFF00) == 0) && ((ret & ((uint64_t)1 << 38)) != 0)) ret |= (128 << 8);

	/* no Safari if Chrome exists */
	if ((ret & ((uint64_t)1 << 39)) != 0) ret = (ret & 0xFFFF00FF) + 0xA300;

	/* windows ce bit */
	if (((ret & 0xFF) == 0) && has_windows_ce) ret |= 128;

	/* windows bit */
	if (((ret & 0xFF) == 0) && has_windows) ret |= 32;

	/* msie bit */
	if (((ret & 0xFF) == 0) && has_msie) ret |= 32;

	return (ret & 0xFFFFFFFF);
}

