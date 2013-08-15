#include <stdio.h>
#include <unistd.h>
#include <coda/cache.hpp>
#include <map>

struct custom_type
{
	custom_type(int v) {}

	size_t estimate_capacity() const
	{
		return 42;
	}
};

int main(int argc, char **argv)
{
	/* TEST 1 */
	{
		printf("Test 1: check that elements in cache with time_max == 0 do not expire\n");

		coda_cache<int, int> cache (1024, 0);
		cache.set(0, 23, false);
		const int *v = cache.get(0);
		printf("right after insert got %d\n", (int) (v ? *v : NULL));

		sleep(1);
		v = cache.get(0);
		printf("one second later got %d\n", (int) (v ? *v : NULL));

		sleep(3);
		v = cache.get(0);
		printf("three more seconds later got %d\n", (int) (v ? *v : NULL));
	}

	/* TEST 2 */
	{
		printf("Test 2: check that elements in cache with time_max = 1 expire after one second\n");

		coda_cache<int, int> cache (1024, 1);
		cache.set(0, 35, false);
		const int *v = cache.get(0);
		printf("right after insert got %d\n", (int) (v ? *v : NULL));

		sleep(1);
		v = cache.get(0);
		printf("one second later got %d\n", (int) (v ? *v : NULL));

		sleep(3);
		v = cache.get(0);
		printf("three more seconds later got %d\n", (int) (v ? *v : NULL));
	}

#if 0
	coda_cache<int, int> cache (1024 * 1024, 86400);

	cache.set(0, 23, false);
	cache.set(1, 32, true);

	std::string dbg;
	cache.dbg(dbg, 1);

	printf("%s\n", dbg.c_str());

#if 1
	// std::vector<long double> v (5);
	std::string v = "12345";

	printf("%u\n", (unsigned) estimate_capacity(v));

	printf("sizeof(std::string)=%u\n", (unsigned) sizeof(std::string));
	printf("sizeof(std::vector<int>)=%u\n", (unsigned) sizeof(std::vector<int>));
	printf("sizeof(std::map<std::string,custom_type>)=%u\n", (unsigned) sizeof(std::map<std::string,custom_type>));
	printf("sizeof(std::map<std::string,custom_type>::value_type)=%u\n", (unsigned) sizeof(std::map<std::string,custom_type>::value_type));
	printf("sizeof(std::pair<std::string,custom_type>)=%u\n", (unsigned) sizeof(std::pair<std::string,custom_type>));
#endif
#endif

	return 0;
}

