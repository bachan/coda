#include <string.h>
#include <coda/arrays.h>
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
	0xfc009fff, /* 1111 1100 0000 0000  1001 1111 1111 1111 */
	            /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
	0x78000001, /* 0111 1000 0000 0000  0000 0000 0000 0001 */
	            /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
	0xb8000001, /* 1011 1000 0000 0000  0000 0000 0000 0001 */
	0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
	0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
	0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
	0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
};

size_t coda_urlenc(u_char* dst, const u_char* src, size_t sz_src)
{
	if (dst == 0) return 0;

	u_char* bdst = dst;

	for (size_t i = 0; i < sz_src; ++i)
	{
		if (table_urlenc[*src >> 5] & (1 << (*src & 0x1f)))
		{
			*dst++ = '%';
			*dst++ = table_hexval[*src >> 4];
			*dst++ = table_hexval[*src & 0xf];
			++src;
		}
		else
		{
			*dst++ = *src++;
		}
	}

	*dst = 0;

	return dst - bdst;
}

size_t coda_urldec(u_char* dst, const u_char* src, size_t sz_src)
{
	if (dst == 0) return 0;

	u_char* bdst = dst;        /* save begin state */
	const u_char* bsrc = src;  /* save begin state */

	while (src < bsrc + sz_src)
	{
		if (*src == '%' && src < bsrc + sz_src - 2)
		{
			*dst++ = table_urldec[*(src+1)] * 0x10 + table_urldec[*(src+2)] * 0x01;
			src += 3;
		}
		else if (*src == '+')
		{
			*dst++ = ' ';
			++src;
		}
		else
		{
			*dst++ = *src++;
		}
	}

	*dst = 0;

	return dst - bdst;
}

