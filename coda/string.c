#include "string.h"

char* coda_strnchr(const char* s, char c, size_t len)
{
	const char* e = s + len;
	for (; s < e; ++s)
	{
		if (c == *s) return (char *) s;
	}
	return NULL;
}

char *coda_strxchr(const char *s, char c, size_t len)
{
	const char *e = s + len;
	for (c = tolower(c); s < e; ++s)
	{
		if (c == tolower(*s)) return (char *) s;
	}
	return NULL;
}

char *coda_revnchr(const char *s, char c, size_t len)
{
	const char *b = s - 1;
	for (s = b + len; s > b; --s)
	{
		if (c == *s) return (char *) s;
	}
	return NULL;
}

char *coda_revxchr(const char *s, char c, size_t len)
{
	const char *b = s - 1;
	for (s = b + len, c = tolower(c); s > b; --s)
	{
		if (c == tolower(*s)) return (char *) s;
	}
	return NULL;
}

size_t coda_strnchp(const char *s, char c, size_t len)
{
	size_t p = 0;
	for (; p < len; ++p)
	{
		if (c == s[p]) return p;
	}
	return len;
}

size_t coda_strxchp(const char *s, char c, size_t len)
{
	size_t p = 0;
	for (c = tolower(c); p < len; ++p)
	{
		if (c == tolower(s[p])) return p;
	}
	return len;
}

char *coda_stristr(const char *s, const char *n)
{
	const char *w;
	const char *x;

	if (!s || !*s) return NULL;
	if (!n || !*n) return (char *) s;

	for (; *s; ++s)
	{
		if (tolower(*s) == tolower(*n))
		{
			for (w = s, x = n; *w && *x; ++w, ++x)
			{
				if (tolower(*w) != tolower(*x)) break;
			}

			if (!*x) return (char *) s;
		}
	}

	return NULL;
}

char *coda_strnstr(const char *s, const char *n, size_t len)
{
	size_t nsz;
	const char *es, *en;
	const char *w;
	const char *x;

	if (!s || !*s) return NULL;
	if (!n || !*n) return (char *) s;

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

char *coda_strxstr(const char *s, const char *n, size_t len)
{
	const char *w;
	const char *x;
	const char *es;
	const char *en;
	size_t nsz;

	if (!s || !*s) return NULL;
	if (!n || !*n) return (char *) s;
	if (len < (nsz = strlen(n))) return NULL;

	es = s + len;
	en = n + nsz;

	for (; s < es; ++s)
	{
		if (tolower(*s) == tolower(*n))
		{
			for (w = s, x = n; w < es && x < en; ++w, ++x)
			{
				if (tolower(*w) != tolower(*x)) break;
			}

			if (x == en) return (char *) s;
		}
	}

	return NULL;
}

/* ~spn */

size_t coda_strispn(const char *s, const char *n)
{
	const char *p = s;
	const char *x;

	for (; *p; ++p)
	{
		for (x = n; *x; ++x)
		{
			if (tolower(*p) == tolower(*x)) break;
		}

		if (!*x) break;
	}

	return p - s;
}

size_t coda_strnspn(const char *s, const char *n, size_t len)
{
	const char *p = s;
	const char *e = s + len;
	const char *x;

	for (; p < e; ++p)
	{
		for (x = n; *x; ++x)
		{
			if (*p == *x) break;
		}

		if (!*x) break;
	}

	return p - s;
}

size_t coda_strxspn(const char *s, const char *n, size_t len)
{
	const char *p = s;
	const char *e = s + len;
	const char *x;

	for (; p < e; ++p)
	{
		for (x = n; *x; ++x)
		{
			if (tolower(*p) == tolower(*x)) break;
		}

		if (!*x) break;
	}

	return p - s;
}

size_t coda_revnspn(const char *s, const char *n, size_t len)
{
	const char *b = s - 1;
	const char *p = s - 1 + len;
	const char *x = n;

	for (; p > b && *x; --p)
	{
		for (x = n; *x; ++x)
		{
			if (*p == *x) break;
		}
	}

	return s - 1 + len - p;
}

size_t coda_revxspn(const char *s, const char *n, size_t len)
{
	const char *b = s - 1;
	const char *p = s - 1 + len;
	const char *x = n;

	for (; p > b && *x; --p)
	{
		for (x = n; *x; ++x)
		{
			if (tolower(*p) == tolower(*x)) break;
		}
	}

	return s - 1 + len - p;
}

/* ~spc */

size_t coda_strispc(const char *s, const char *n)
{
	const char *p = s;
	const char *x;

	for (; *p; ++p)
	{
		for (x = n; *x; ++x)
		{
			if (tolower(*p) == tolower(*x)) return p - s;
		}
	}

	return p - s;
}

size_t coda_strnspc(const char *s, const char *n, size_t len)
{
	const char *p = s;
	const char *e = s + len;
	const char *x;

	for (; p < e; ++p)
	{
		for (x = n; *x; ++x)
		{
			if (*p == *x) return p - s;
		}
	}

	return p - s;
}

size_t coda_strxspc(const char *s, const char *n, size_t len)
{
	const char *p = s;
	const char *e = s + len;
	const char *x;

	for (; p < e; ++p)
	{
		for (x = n; *x; ++x)
		{
			if (tolower(*p) == tolower(*x)) return p - s;
		}
	}

	return p - s;
}

size_t coda_revnspc(const char *s, const char *n, size_t len)
{
	const char *b = s - 1;
	const char *p = s - 1 + len;
	const char *x;

	for (; p > b; --p)
	{
		for (x = n; *x; ++x)
		{
			if (*p == *x) return s - 1 + len - p;
		}
	}

	return s - 1 + len - p;
}

size_t coda_revxspc(const char *s, const char *n, size_t len)
{
	const char *b = s - 1;
	const char *p = s - 1 + len;
	const char *x;

	for (; p > b; --p)
	{
		for (x = n; *x; ++x)
		{
			if (tolower(*p) == tolower(*x)) return s - 1 + len - p;
		}
	}

	return s - 1 + len - p;
}

