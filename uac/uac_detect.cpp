#include <stdio.h>
#include "uac.h"

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("usage: uac_detect uac.automaton config.xml\n");
		return -1;
	}

	uac_init(argv[1]);
	uac_texts_init(argv[2]);

	while (!feof(stdin))
	{
		char x[1024];
		if (fgets(x, 1024, stdin))
		{
			uint32_t ua = uac_determine(x);
/*			if ((ua & 0xFF00) == 0) printf("%s", x);*/
			printf("%08X: %s%s, %s\n\n", ua, x, uac_texts_os(ua & 0xFF), uac_texts_browser((ua >> 8) & 0xFF));
		}
	}

	uac_free();
	uac_texts_free();

	return 0;
}

