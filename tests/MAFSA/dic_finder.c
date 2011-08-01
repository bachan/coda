#include <stdio.h>
#include <string.h>
#include <MAFSA/automaton.h>
#include "dic.h"

MAFSA_automaton a;

void dic_search_word(const char *word)
{
	MAFSA_letter out [MAX_OUT];
	MAFSA_letter tmp [MAX_OUT];
	size_t out_sz;
	int value;

	out_sz = SIMPLE_conv_string_to_letters(word, out, MAX_OUT);

	if (1 != MAFSA_automaton_enumerate_int(a, out, out_sz, tmp, MAX_OUT, SIMPLE_LETTER_DELIM, &value, 1))
	{
		printf("%s not found\n", word);
	}
	else
	{
		printf("%s\t%d\n", word, value);
	}
}

int main(int argc, char **argv)
{
	if (2 > argc)
	{
		fprintf(stderr, "Usage: %s dictionary.dic\n", argv[0]);
		return -1;
	}

	a = MAFSA_automaton_load_from_binary_file(argv[1], NULL);
	if (NULL == a) return -1;

	while (!feof(stdin) && !ferror(stdin))
	{
		char text [4096];
		size_t size;

		if (NULL == fgets(text, 4096, stdin))
		{
			break;
		}

		size = strcspn(text, "\r\n");
		text[size] = 0;

		dic_search_word(text);
	}

	MAFSA_automaton_close(a);

	return 0;
}

