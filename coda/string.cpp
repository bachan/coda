#include <stdio.h>
#include <stdarg.h>
#include "string.hpp"

#define BUFSZ 32768

int coda_strappend(std::string &out, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	int bytes;
	char BUF [BUFSZ];

	if (BUFSZ <= (bytes = vsnprintf(BUF, BUFSZ, fmt, ap)))
	{
		bytes = BUFSZ - 1;
	}

	out.append(BUF, bytes);

	return bytes;
}

int coda_strnappend(std::string &out, size_t n, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	int bytes;

	if (n < BUFSZ)
	{
		char BUF [BUFSZ];

		if ((int) n < (bytes = vsnprintf(BUF, n + 1, fmt, ap)))
		{
			bytes = n;
		}

		out.append(BUF, bytes);
	}
	else
	{
		char *BUF = new char [n + 1];

		if ((int) n < (bytes = vsnprintf(BUF, n + 1, fmt, ap)))
		{
			bytes = n;
		}

		out.append(BUF, bytes);

		delete [] BUF;
	}

	return bytes;
}

int coda_strnprintf(std::string &out, size_t pos, size_t n, const char *fmt, ...)
{
	if (out.size() < pos) return -1;

	va_list ap;
	va_start(ap, fmt);

	int bytes;

	if (n < BUFSZ)
	{
		char BUF [BUFSZ];

		if ((int) n < (bytes = vsnprintf(BUF, n + 1, fmt, ap)))
		{
			bytes = n;
		}

		out.insert(pos, BUF, bytes);
	}
	else
	{
		char *BUF = new char [n + 1];

		if ((int) n < (bytes = vsnprintf(BUF, n + 1, fmt, ap)))
		{
			bytes = n;
		}

		out.insert(pos, BUF, bytes);

		delete [] BUF;
	}

	return bytes;
}

