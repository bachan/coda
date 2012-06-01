#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "string.hpp"

#define BUFSZ 32768

int coda_strappend(std::string& out, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	int bytes;
	char BUF[BUFSZ];

	if (BUFSZ <= (bytes = vsnprintf(BUF, BUFSZ, fmt, ap)))
	{
		bytes = BUFSZ - 1;
	}

	out.append(BUF, bytes);

	return bytes;
}

int coda_strnappend(std::string& out, size_t num, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	int bytes;

	if (num < BUFSZ)
	{
		char BUF [BUFSZ];

		if ((int) num < (bytes = vsnprintf(BUF, num + 1, fmt, ap)))
		{
			bytes = num;
		}

		out.append(BUF, bytes);
	}
	else
	{
		char* BUF = new char [num + 1];

		if ((int) num < (bytes = vsnprintf(BUF, num + 1, fmt, ap)))
		{
			bytes = num;
		}

		out.append(BUF, bytes);

		delete [] BUF;
	}

	return bytes;
}

int coda_strnprintf(std::string& out, size_t pos, size_t num, const char* fmt, ...)
{
	if (out.size() < pos) return -1;

	va_list ap;
	va_start(ap, fmt);

	int bytes;

	if (num < BUFSZ)
	{
		char BUF[BUFSZ];

		if ((int) num < (bytes = vsnprintf(BUF, num + 1, fmt, ap)))
		{
			bytes = num;
		}

		out.insert(pos, BUF, bytes);
	}
	else
	{
		char* BUF = new char [num + 1];

		if ((int) num < (bytes = vsnprintf(BUF, num + 1, fmt, ap)))
		{
			bytes = num;
		}

		out.insert(pos, BUF, bytes);

		delete [] BUF;
	}

	return bytes;
}

void coda_get_stringset(const char* str, std::set<std::string>& result)
{
	if (!str || !*str)
	{
		return;
	}

	const char* begin = str;
	const char* i = str;
	while (i)
	{
		i = strchr(i, ',');
		if (i)
		{
			std::string temp(begin, i - begin);
			result.insert(temp);
			++i;
		}
		else
		{
			std::string temp(begin);
			result.insert(temp);
		}
		begin = i;
	}
}

void coda_get_uint64set(const char* str, std::set<uint64_t>& result)
{
	const char* i = str;
	while (i)
	{
		uint64_t temp;
		sscanf(i, "%"PRIu64, &temp);
		result.insert(temp);
		i = strchr(i, ',');
		if (i)
		{
			++i;
		}
	}
}
