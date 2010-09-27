#include "arrays.h"

char* coda_strnchr(const char* s, char c, size_t len)
{
	const char* e = s + len;
	for (; s < e; ++s)
	{
		if (c == *s) return (char *) s;
	}
	return NULL;
}

char* rdev_strnstr(const char* s, const char* n, size_t len)
{
	size_t nsz;
	const char* es;
	const char* en;
	const char* w;
	const char* x;

	if (!s || !*s) return NULL;
	if (!n || !*n) return (char*) s;

	if (len < (nsz = strlen(n))) return NULL;

	es = s + len;
	en = n + nsz;

	for (; s < es; ++s)
	{
		if (*s == *n)
		{
			for (w = s, x = n; w < es && x < en; ++w, ++x)
			{
				if (*w != *x) break;
			}

			if (x == en) return (char *) s;
		}
	}

	return NULL;
}

