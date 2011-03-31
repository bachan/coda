#include <stdlib.h>
#include <MAFSA/daciuk.hpp>
#include "dic.h"

MAFSA::daciuk<SIMPLE_LETTER_DELIM> dict;

void dic_insert_word(const char *word, unsigned int value)
{
	MAFSA_letter out [MAX_OUT];
	size_t out_sz;

	out_sz = SIMPLE_conv_string_to_letters(word, out, MAX_OUT);
	out[out_sz++] = SIMPLE_LETTER_DELIM;

	while (value)
	{
		out[out_sz++] = value % SIMPLE_LETTER_DELIM;
		value /= SIMPLE_LETTER_DELIM;
	}

	dict.insert(out, out_sz);
}

int main(int argc, char **argv)
{
	if (2 > argc)
	{
		fprintf(stderr, "Usage: %s dictionary.dic\n", argv[0]);
		return -1;
	}

	while (!feof(stdin) && !ferror(stdin))
	{
		char text [4096];
		size_t size, tabs;

		if (NULL == fgets(text, 4096, stdin))
		{
			break;
		}

		size = strcspn(text, "\r\n");
		text[size] = 0;

		tabs = strcspn(text, "\t");
		text[tabs] = 0;

		dic_insert_word(text, strtoul(text + tabs + 1, NULL, 10));
	}

	dict.save_to_file(argv[1]);

	return 0;
}

