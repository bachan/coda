#include <stdio.h>
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
	coda_cache<int, int> cache (1024 * 1024, 86400);

	cache.set(0, 23, false);
	cache.set(1, 32, true);

	std::string dbg;
	cache.dbg(dbg, 1);

	printf("%s\n", dbg.c_str());

#if 0
	// std::vector<long double> v (5);
	std::string v = "12345";

	printf("%u\n", (unsigned) estimate_capacity(v));

	printf("sizeof(std::string)=%u\n", sizeof(std::string));
	printf("sizeof(std::vector<int>)=%u\n", sizeof(std::vector<int>));
	printf("sizeof(std::map<std::string,custom_type>)=%u\n", sizeof(std::map<std::string,custom_type>));
	printf("sizeof(std::map<std::string,custom_type>::value_type)=%u\n", sizeof(std::map<std::string,custom_type>::value_type));
	printf("sizeof(std::pair<std::string,custom_type>)=%u\n", sizeof(std::pair<std::string,custom_type>));
#endif

	return 0;
}

