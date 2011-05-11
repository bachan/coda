#include "dic.h"

size_t SIMPLE_conv_string_to_letters(const char *s, MAFSA_letter *l, size_t sz_l)
{
	size_t pos;
	for (pos = 0; *s && pos < sz_l; ++s, ++pos)
	{
		char c = *s;

		if ('-' == c)
		{
			l[pos] = SIMPLE_LETTER_DEFIS;
		}
		else if ('\'' == c)
		{
			l[pos] = SIMPLE_LETTER_APOSTROPHE;
		}
		else if (' ' == c)
		{
			l[pos] = SIMPLE_LETTER_SPACE;
		}
		else
		{
			if ('A' <= c && c <= 'Z')
			{
				l[pos] = c - 'A';
			}
			else if ('a' <= c && c <= 'z')
			{
				l[pos] = c - 'a';
			}
			else
			{
				break;
			}
		}
	}

	return pos;
}

size_t SIMPLE_conv_letters_to_string(const MAFSA_letter *l, size_t sz_l, char *s, size_t sz_s)
{
	size_t pos;
	for (pos = 0; pos + 1 < sz_s && pos < sz_l; ++pos)
	{
		switch (l[pos])
		{
		case SIMPLE_LETTER_DEFIS:
			s[pos] = '-';
			break;
		case SIMPLE_LETTER_APOSTROPHE:
			s[pos] = '\'';
			break;
		case SIMPLE_LETTER_SPACE:
			s[pos] = ' ';
			break;
		case SIMPLE_LETTER_DELIM:
			s[pos] = '|';
			break;
		default: s[pos] = 'A' + l[pos]; break;
		}
	}
	s[pos] = 0;
	return pos;
}

