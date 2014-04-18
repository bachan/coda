#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "string.h"
#include "url.hpp"

int coda_get_params_buf(coda_params_t& params, const char* s, size_t len)
{
	if (!s) return -1;

	const char* e = s + len;

	while (s < e)
	{
		const char* amp = coda_strnchr(s, '&', e - s);
		if (NULL == amp) amp = e;

		const char* equ = coda_strnchr(s, '=', e - s);
		if (NULL != equ && equ < amp)
		{
			std::string key (s, equ - s);
			params[key].assign(equ + 1, amp - equ - 1);
		}

		s = amp + 1;
	}

	return 0;
}

int coda_get_header_buf(coda_header_t& header, const char* s, size_t len)
{
	if (!s) return -1;

	const char* e = s + len;

	while (s < e)
	{
		const char* crlf = coda_strnstr(s, "\r\n", e - s);
		if (NULL == crlf) crlf = e;
		if (crlf == s) break; /* \r\n\r\n situation, end of headers */

		const char* dodo = coda_strnchr(s, ':', e - s);
		if (NULL != dodo && dodo < crlf)
		{
			std::string key (s, dodo - s);
			header[key].assign(dodo + 1, crlf - dodo - 1);
		}

		s = crlf + 2;
	}

	return 0;
}

int coda_get_params(coda_params_t &params, const char *s)
{
	if (!s) return -1;
	return coda_get_params_buf(params, s, strlen(s));
}

int coda_get_header(coda_header_t &header, const char *s)
{
	if (!s) return -1;
	return coda_get_header_buf(header, s, strlen(s));
}

static u_char table_hexval [] = "0123456789ABCDEF";

static u_char table_urldec [UCHAR_MAX + 1] = 
{
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0,  /* 0-9 */
	0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* a-f */
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* A-F */
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

static uint32_t table_urlenc [] = 
{
	0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
	            /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
	0xfc009ffe, /* 1111 1100 0000 0000  1001 1111 1111 1110 */
	            /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
	0x78000001, /* 0111 1000 0000 0000  0000 0000 0000 0001 */
	            /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
	0xb8000001, /* 1011 1000 0000 0000  0000 0000 0000 0001 */
	0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
	0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
	0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
	0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
};

size_t coda_urlenc(char* dst, const char* src, size_t sz_src)
{
	unsigned char* pdst = (unsigned char *) dst;
	unsigned char* psrc = (unsigned char *) src;
	unsigned char* esrc = (unsigned char *) src + sz_src;

	while (psrc < esrc)
	{
		if (table_urlenc[*psrc >> 5] & (1 << (*psrc & 0x1f)))
		{
			*pdst++ = '%';
			*pdst++ = table_hexval[*psrc >> 4];
			*pdst++ = table_hexval[*psrc & 0xf];
			++psrc;
		}
		else if (*psrc == ' ')
		{
			*pdst++ = '+';
			++psrc;
		}
		else
		{
			*pdst++ = *psrc++;
		}
	}

	*pdst = 0;

	return pdst - (unsigned char *) dst;
}

size_t coda_urldec(char* dst, const char* src, size_t sz_src)
{
	unsigned char* pdst = (unsigned char *) dst;
	const unsigned char* psrc = (const unsigned char *) src;
	const unsigned char* esrc = (const unsigned char *) src + sz_src;

	while (psrc < esrc)
	{
		if (*psrc == '%' && psrc < esrc - 2)
		{
			*pdst++ = table_urldec[*(psrc+1)] * 0x10 + table_urldec[*(psrc+2)] * 0x01;
			psrc += 3;
		}
		else if (*psrc == '+')
		{
			*pdst++ = ' ';
			++psrc;
		}
		else
		{
			*pdst++ = *psrc++;
		}
	}

	*pdst = 0;

	return pdst - (unsigned char *) dst;
}

std::string coda_urlenc(const char* src, size_t sz_src)
{
	std::string dst;

	unsigned char* psrc = (unsigned char *) src;
	unsigned char* esrc = (unsigned char *) src + sz_src;

	while (psrc < esrc)
	{
		if (table_urlenc[*psrc >> 5] & (1 << (*psrc & 0x1f)))
		{
			dst.push_back('%');
			dst.push_back(table_hexval[*psrc >> 4]);
			dst.push_back(table_hexval[*psrc & 0xf]);
			++psrc;
		}
		else if (*psrc == ' ')
		{
			dst.push_back('+');
			++psrc;
		}
		else
		{
			dst.push_back(*psrc++);
		}
	}

	return dst;
}

std::string coda_urldec(const char* src, size_t sz_src)
{
	std::string dst;

	const unsigned char* psrc = (const unsigned char *) src;
	const unsigned char* esrc = (const unsigned char *) src + sz_src;

	while (psrc < esrc)
	{
		if (*psrc == '%' && psrc < esrc - 2)
		{
			dst.push_back(table_urldec[*(psrc+1)] * 0x10 + table_urldec[*(psrc+2)] * 0x01);
			psrc += 3;
		}
		else if (*psrc == '+')
		{
			dst.push_back(' ');
			++psrc;
		}
		else
		{
			dst.push_back(*psrc++);
		}
	}

	return dst;
}

void coda_url_escape(const char *s, char *dest, size_t sz)
{
	memset(dest, 0, sz);
	size_t ptr = 0;
	sz -= 3;
	while (*s && (ptr < sz))
	{
		//if (*s & 0x80 != 0)
		if (table_urlenc[*s >> 5] & (1 << (*s & 0x1f)))
		{
			ptr += snprintf(dest + ptr, sz - ptr, "%%%02X", *s);
		}
		else if (*s == ' ')
		{
			dest[ptr++] = '+';
		}
		else
		{
			dest[ptr++] = *s;
		}

		s++;
	}
}

void coda_url_unescape(const char *s, char *dest, size_t sz)
{
	memset(dest, 0, sz);
	size_t ptr = 0;
	while (*s && (ptr < sz - 2))
	{
		if (*s == '+')
		{
			dest[ptr++] = ' ';
			s++;
		}
		else if ((*s == '%') && isxdigit(s[1]) && isxdigit(s[2]))
		{
			char xnum[3];
			xnum[0] = s[1];
			xnum[1] = s[2];
			xnum[2] = 0;
			uint8_t c = strtol(xnum, 0, 16);
			dest[ptr] = (char)c;
			ptr++;
			s += 3;
		}
		else if ((*s == '%') && (s[1] == 'u') && isxdigit(s[2]))
		{
			char *ns;
			uint32_t c = strtol(s + 2, &ns, 16);
			s = ns;
			/* doing only russian letters */
			if ((c < 0x0800) && (c > 0x7F))
			{
				dest[ptr++] = ((c >> 6) | 0xc0);
				dest[ptr++] = ((c & 0x3f) | 0x80);
			}
			else if (c < 0x7F)
			{
				dest[ptr++] = c;
			}
		}
		else if ((*s == '\\') && (s[1] == 'x') && isxdigit(s[2]) && isxdigit(s[3]))
		{
			char xnum[3];
			xnum[0] = s[2];
			xnum[1] = s[3];
			xnum[2] = 0;
			uint8_t c = strtol(xnum, 0, 16);
			dest[ptr] = (char)c;
			ptr++;
			s += 4;
		}
		else
		{
			dest[ptr] = *s;
			ptr++;
			s++;
		}
	}
	dest[ptr] = 0;
}

void coda_url_preunescape(const char* s, char* dest, size_t sz)
{
	size_t ptr = 0;
	while (*s && (ptr < sz + 5))
	{
		if ((*s == '%') && isxdigit(s[1]) && isxdigit(s[2]))
		{
			char xnum[3];
			xnum[0] = s[1];
			xnum[1] = s[2];
			xnum[2] = 0;
			uint8_t c = strtol(xnum, 0, 16);
			if (c & 0x80)
			{
				dest[ptr] = (char)c;
				ptr++;
			}
			else
			{
				if (ptr + 4 < sz) memcpy(dest + ptr, s, 3);
				ptr += 3;
			}
			s += 3;
		}
		else if ((*s == '\\') && (s[1] == 'x') && isxdigit(s[2]) && isxdigit(s[3]))
		{
			char xnum[3];
			xnum[0] = s[2];
			xnum[1] = s[3];
			xnum[2] = 0;
			uint8_t c = strtol(xnum, 0, 16);
			if (c & 0x80)
			{
				dest[ptr] = (char)c;
				ptr++;
			}
			else
			{
				if (ptr + 3 < sz) memcpy(dest + ptr, s, 4);
			}
			s += 4;
		}
		else
		{
			dest[ptr] = *s;
			ptr++;
			s++;
		}
	}
	dest[ptr] = 0;
}

