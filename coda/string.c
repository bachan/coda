#include "string.h"

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

