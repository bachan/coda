#include <stdio.h>
#include <stdint.h>
#include <coda/stringstorage.h>

static void mycb(void *user_data, const char *key, void *value)
{
	printf("%s:%u\n", key, *(uint32_t*)value);
}

int main()
{
	string_storage ss;
	string_storage_init(&ss, sizeof(uint32_t));

	uint32_t* A;
	A = string_storage_add(&ss, "AAA"); *A = 5;
	A = string_storage_add(&ss, "AAAaaaaaaa"); *A = 25;
	A = string_storage_add(&ss, "AAAaaaaabbbb"); *A = 26;

	string_storage_enumerate(&ss, mycb, 0);
	string_storage_free(&ss);

	return 0;
}


