#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "string.h"
#include "url_parser.hpp"

coda_url::coda_url()
{
	val_str = dom_str = NULL;
	val_len = dom_len = 0;
}

coda_url::coda_url(const coda_url &u)
{
	if (u.empty())
	{
		val_len = 0;
		dom_len = 0;

		val_str = NULL;
		dom_str = NULL;
	}
	else
	{
		val_len = u.val_len;
		dom_len = u.dom_len;

		val_str = (char *) malloc(val_len + dom_len + 2);
		memcpy(val_str, u.val_str, val_len + dom_len + 2);

		dom_str = val_str + val_len + 1;
	}
}

coda_url &coda_url::operator= (const coda_url &u)
{
	if (allocated())
	{
		free(val_str);
	}

	if (u.empty())
	{
		val_len = 0;
		dom_len = 0;

		val_str = NULL;
		dom_str = NULL;
	}
	else
	{
		val_len = u.val_len;
		dom_len = u.dom_len;

		val_str = (char *) malloc(val_len + dom_len + 2);
		memcpy(val_str, u.val_str, val_len + dom_len + 2);

		dom_str = val_str + val_len + 1;
	}

	return *this;
}

coda_url::~coda_url()
{
	if (allocated())
	{
		free(val_str);
	}
}

int coda_url::reset()
{
	if (allocated())
	{
		free(val_str);
	}

	val_str = dom_str = NULL;
	val_len = dom_len = 0;

	return 0;
}

int coda_url::create_absolute(const char *u, size_t sz, uint32_t flags)
{
	if (!empty() || 0 == sz || NULL == u)
	{
		reset();
		return -1;
	}

	val_str = (char *) malloc(2 + 2 * sz + 9); // val + 0 + dom + 0 + "http[s]://" + "/"

	create_scheme(u, sz, flags);

	return 0;
}

int coda_url::create_relative(const char *u, size_t sz, uint32_t flags, const coda_url &parent)
{
	if (!empty() || 0 == sz || NULL == u || parent.empty())
	{
		reset();
		return -1;
	}

	val_str = (char *) malloc(2 + 2 * sz + parent.val_len + parent.dom_len); // if relative, parent may be bigger than u

	if (coda_strxstr(u, "http://", 7) || coda_strxstr(u, "https://", 8))
	{
		create_scheme(u, sz, flags);

		if ((flags & CODA_URL_SUBDOMAIN) && 0 != strcasecmp(dom_str, parent.dom_str))
		{
			reset();
			return -1;
		}
	}
	else
	{
		if (-1 == create_suburl(u, sz, flags, parent))
		{
			reset();
			return -1;
		}
	}

	return 0;
}

int coda_url::create_scheme(const char *u, size_t sz, uint32_t flags)
{
	if (coda_strxstr(u, "https://", 8))
	{
		val_len += coda_cpymsz(val_str, u, sz);
		return assign_anchor(val_str + 8, flags);
	}

	if (coda_strxstr(u, "http://", 7))
	{
		val_len += coda_cpymsz(val_str, u, sz);
		return assign_anchor(val_str + 7, flags);
	}

	val_len = coda_cpymsz(val_str, "http://", sizeof("http://") - 1);
	val_len = coda_cpymsz(val_str + val_len, u, sz) + val_len;
	return assign_anchor(val_str + 7, flags);
}

#define DOMAIN_SYMBOL(c) (('a' <= (c) && (c) <= 'z') \
					   || ('A' <= (c) && (c) <= 'Z') \
					   || ('0' <= (c) && (c) <= '9') \
					   || ('.' == (c) || ':' == (c)) \
					   || ('-' == (c) || '_' == (c)))

int coda_url::assign_anchor(char *beg, uint32_t flags)
{
	char *p;
	char *end;

	if (NULL != (end = coda_strnchr(val_str, '#', val_len)))
	{
		val_len = end - val_str;
	}

	end = coda_strnchr(beg, '/', val_str + val_len - beg);
	if (NULL == end) end = val_str + val_len;

	for (p = beg; p < end; ++p)
	{
		*p = tolower(*p);

		if (!DOMAIN_SYMBOL(*p))
		{
			break;
		}
	}

	if (end == p)
	{
		if (val_str + val_len == end)
		{
			end = val_str + val_len;
			val_str[val_len++] = '/';
		}
	}
	else
	{
		memmove(p + 1, p, val_str + val_len - p);
		end = p; p[0] = '/'; val_len++;
	}

	if (beg != (p = beg + coda_strnspn(beg, ".", end - beg)))
	{
		memmove(beg, p, val_str + val_len - p);
		val_len -= p - beg;
			end -= p - beg;
	}

	if (end != (p = end - coda_revnspn(beg, ".", end - beg)))
	{
		memmove(p + 1, end, val_str + val_len - end);
		val_len -= end - (p + 1);
			end -= end - (p + 1);
	}

	p = end;

	if (NULL == (p = coda_revnchr(beg, '.', (p - 1) - beg))) p = beg - 1;
	if (NULL == (p = coda_revnchr(beg, '.', (p - 1) - beg))) p = beg - 1;

	dom_str = p + 1;
	dom_len = end - (p + 1);

	return assign_refers(end, flags);
}

int coda_url::assign_refers(char *beg, uint32_t flags)
{
	char *eofp = coda_strnchr(val_str, '?', val_len);
	if (NULL == eofp) eofp = val_str + val_len;

	char *args = eofp;
	size_t args_len = val_str + val_len - args;

	char *p = beg;
	char *prev = p;

	while (p < eofp)
	{
		if (*p == '/')
		{
			if (p <= eofp - 4 && coda_strnstr(p, "/../", 4) != 0) // /foo/../bar -> /bar
			{
				memmove(prev, p + 3, eofp - (p + 3));
				eofp -= p + 3 - prev;

				p = prev;

				for (char *pp = p - 1; pp >= beg; --pp)
				{
					if (*pp == '/')
					{
						prev = pp;
						break;
					}
				}
			}
			else if (p <= eofp - 3 && coda_strnstr(p, "/./", 3) != 0) // /foo/./bar -> /foo/bar
			{
				memmove(p, p + 2, eofp - (p + 2));
				eofp -= 2;
			}
#if 1
			/*
			 * mozilla & opera thinks that // is directory with empty name
			 * but in unix-paths // == /./ let's think as mozilla, but just comment 
			 * this code for potential future usage.
			 */
			else if (p <= eofp - 2 && coda_strnstr(p, "//", 2) != 0) // /foo//bar -> /foo/bar
			{
				memmove(p, p + 1, eofp - (p + 1));
				eofp -= 1;
			}
#endif
			else
			{
				prev = p;
				++p;
			}
		}
		else
		{
			++p;
		}
	}

	if (prev == eofp - 3 && NULL != coda_strnstr(prev, "/..", 3))
	{
		if (prev > beg)
		{
			for (char *pp = prev - 1; pp >= beg; --pp)
			{
				if (*pp == '/')
				{
					if (pp > beg && !(flags & CODA_URL_FETCHABLE))
					{
						eofp -= prev + 3 - pp;
					}
					else
					{
						eofp -= prev + 3 - (pp + 1); // +1 чтоб слеш в конце остался
					}
					break;
				}
			}
		}
		else // если мы в корне, то выше ничего нет
		{
			eofp -= 2; // 2, a не 3, чтоб слеш в конце остался
		}
	}
	else if (prev == eofp - 2 && NULL != coda_strnstr(prev, "/.", 2))
	{
		if (prev > beg && !(flags & CODA_URL_FETCHABLE))
		{
			eofp -= 2;
		}
		else
		{
			eofp -= 1; // 1, а не 2, чтоб слеш в конце остался
		}
	}
	else if (prev == eofp - 1 && NULL != coda_strnstr(prev, "/", 1))
	{
		if (prev > beg && !(flags & CODA_URL_FETCHABLE))
		{
			eofp -= 1;
		}
	}

	if (args_len != 0) // наконец меммувим аргументы
	{
		memmove(eofp, args, args_len);
		eofp += args_len;
	}

	val_len = eofp - val_str;

	dom_len = coda_cpymsz(val_str + val_len + 1, dom_str, dom_len);
	dom_str = val_str + val_len + 1;

	val_str[val_len] = 0;
	dom_str[dom_len] = 0;

	return 0;
}

int coda_url::create_suburl(const char *u, size_t sz, uint32_t flags, const coda_url &parent)
{
	if (NULL != coda_strxstr(u, "mailto:", 7)) return -1;
	if (NULL != coda_strxstr(u, "javascript:", 11)) return -1;
	if ( '#' == *u) return -1;

	size_t sz_dom = (parent.val_str[4] != 's' ? 7 : 8);
	size_t sz_pub = sz_dom = coda_strnchr(parent.val_str + sz_dom,
		'/', parent.val_len - sz_dom) - parent.val_str;

	char *p = coda_strnchr(u, '#', sz);
	if (NULL != p) sz = p - u;

	if ('/' != *u)
	{
		sz_pub = coda_strnchp(parent.val_str, '?', parent.val_len);
		sz_pub = coda_revnchr(parent.val_str, '/', sz_pub) - parent.val_str + 1;
	}

	val_len = coda_cpymsz(val_str, parent.val_str, sz_pub)
			+ coda_cpymsz(val_str + sz_pub, u, sz);

	dom_len = parent.dom_len;
	dom_str = parent.dom_str;

	return assign_refers(val_str + sz_dom, flags);
}

