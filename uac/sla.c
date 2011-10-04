/*
 * Copyright (C) 2007, libturglem development team.
 *
 * This file is released under the LGPL.
*/

#include "sla.h"
#include <string.h>

ssize_t conv_s2l_sla(const char *s, MAFSA_letter *l, size_t sz)
{
	ssize_t pos = 0;
	const char *i = s;
	while (*i && (*i != '\n') && (pos < sz))
	{
		if (i[0] >= '0' && i[0] <= '9')
		{
			l[pos++] = i[0] - '0';
			i++;
		}
		else if (strchr(" \t_@./+\\\"'=%$#@!?(){}[]", i[0]))
		{
			if (pos && (l[pos-1] != 10) && (l[pos-1] != 11)) l[pos++] = 10;
			i++;
		}
		else if (strchr("-!#$%^&*().\\", i[0]))
		{
			if (pos && (l[pos-1] != 10) && (l[pos-1] != 11)) l[pos++] = 11; /* '-' */
			i++;
		}
		else if (i[0] >= 'a' && i[0] <= 'z')
		{
			l[pos++] = i[0] - 'a' + 12;
			i++;
		}
		else if (i[0] >= 'A' && i[0] <= 'Z')
		{
			l[pos++] = i[0] - 'A' + 12;
			i++;
		}
		else if (i[0] == '|')
		{
			l[pos++] = MAX_LETTER_SLA;
			i++;
		}
		else i++;
	}
	return pos;
}

size_t conv_l2s_sla(const MAFSA_letter *l, size_t sz_l, char *s, size_t sz_s)
{
	size_t pos_w = 0;
	size_t pos_r = 0;

	while ((pos_w + 1 < sz_s) && (pos_r < sz_l))
	{
		if(l[pos_r] < 10)
		{
			s[pos_w++] = l[pos_r] + '0';
		}
		else if ((l[pos_r] < 38) && (l[pos_r] > 11))
		{
			s[pos_w++] = l[pos_r] - 12 + 'A';
		}
		else if (pos_w)
		{
			switch (l[pos_r])
			{
				case 10:
					s[pos_w++] = '_';
					break;
				case 11:
					s[pos_w++] = '-';
					break;
				default:
					s[pos_w++] = '|';
					break;
			}
		}
		pos_r++;
	}
	s[pos_w] = 0;
	return pos_w;
}

ssize_t conv_b2l_sla(const char *s, size_t sz_s, MAFSA_letter *l, size_t sz_l)
{
	ssize_t pos = 0;
	const char *i = s;
	const char *e = s + sz_s;
	while ((i < e) && (*i != '\n') && (pos < sz_l))
	{
		if (i[0] >= '0' && i[0] <= '9')
		{
			l[pos++] = i[0] - '0';
			i++;
		}
		else if (strchr(" \t_@./+\\\"'=%$#@!?(){}[]", i[0]))
		{
			if (pos && (l[pos-1] != 10) && (l[pos-1] != 11)) l[pos++] = 10;
			i++;
		}
		else if (strchr("-!#$%^&*().\\", i[0]))
		{
			if (pos && (l[pos-1] != 10) && (l[pos-1] != 11)) l[pos++] = 11; /* '-' */
			i++;
		}
		else if (i[0] >= 'a' && i[0] <= 'z')
		{
			l[pos++] = i[0] - 'a' + 12;
			i++;
		}
		else if (i[0] >= 'A' && i[0] <= 'Z')
		{
			l[pos++] = i[0] - 'A' + 12;
			i++;
		}
		else if (i[0] == '|')
		{
			l[pos++] = MAX_LETTER_SLA;
			i++;
		}
		else i++;
	}
	return pos;
}

size_t conv_l2b_sla(const MAFSA_letter *l, size_t sz_l, char *s, size_t sz_s)
{
	size_t pos_w = 0;
	size_t pos_r = 0;

	while ((pos_w < sz_s) && (pos_r < sz_l))
	{
		if(l[pos_r] < 10)
		{
			s[pos_w++] = l[pos_r] + '0';
		}
		else if ((l[pos_r] < 38) && (l[pos_r] > 11))
		{
			s[pos_w++] = l[pos_r] - 12 + 'A';
		}
		else if (pos_w)
		{
			switch (l[pos_r])
			{
				case 10:
					s[pos_w++] = '_';
					break;
				case 11:
					s[pos_w++] = '-';
					break;
				default:
					s[pos_w++] = '|';
					break;
			}
		}
		pos_r++;
	}
	return pos_w;
}

