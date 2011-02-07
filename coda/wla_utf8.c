/*
 * Copyright (C) 2007, libturglem development team.
 *
 * This file is released under the LGPL.
*/

#include <string.h>
#include "wla_utf8.h"

ssize_t conv_s2l_wla_utf8(const char *s, MAFSA_letter *l, size_t sz)
{
	u_int32_t cur_letter;
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
		else if (((i[0] & 0xe0) == 0xc0) && ((i[1] & 0xc0) == 0x80))
		{
			cur_letter = ((i[0] & 0x1f) << 6) | (i[1] & 0x3f);

			if ((cur_letter >= 1040) && (cur_letter <= 1071))
			{
				l[pos] = cur_letter - 1040 + 38;
			}
			else
			if ((cur_letter >= 1072) && (cur_letter <= 1103))
			{
				l[pos] = cur_letter - 1072 + 38;
			}
			else
			if ((cur_letter == 1025) || (cur_letter == 1105))
			{
				l[pos] = 5 + 38;
			}
			else
			{
				/*return -1;*/
			}
			i += 2;
			pos++;
		}
		else if (i[0] == '|')
		{
			l[pos++] = 70;
			i++;
		}
		else i++;
	}
	return pos;
}

size_t conv_l2s_wla_utf8(const MAFSA_letter *l, size_t sz_l, char *s, size_t sz_s)
{
	size_t pos_w = 0;
	size_t pos_r = 0;
	u_int32_t c;

	while ((pos_w + 1 < sz_s) && (pos_r < sz_l))
	{
		if(l[pos_r] < 10)
		{
			s[pos_w++] = l[pos_r] + '0';
		}
		else if ((l[pos_r] < 38) && (l[pos_r] > 11))
		{
			s[pos_w++] = l[pos_r] - 12 + 'a';
		}
		else if ((l[pos_r] < 70) && (l[pos_r] > 37))
		{
			if (pos_w + 2 >= sz_s) break;
			c = l[pos_r] - 38 + 1072;
			s[pos_w] = (c >> 6) | 0xc0;
			s[pos_w + 1] = (c & 0x3f) | 0x80;
			pos_w += 2;
		}
		else if (pos_w && (s[pos_w-1] != '_') && (s[pos_w-1] != '-'))
		{
			switch (l[pos_r])
			{
				case 10:
					s[pos_w++] = ' ';
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

