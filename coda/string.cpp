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

bool coda_string_replace(std::string& str, const char* pattern, const char* replacement)
{
	size_t pos = str.find(pattern);

	if (std::string::npos == pos)
	{
		return false;
	}

	str.replace(pos, strlen(pattern), replacement);

	return true;
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

void coda_get_uint32set(const char* str, std::set<unsigned int>& result)
{
	const char* i = str;
	while (i)
	{
		unsigned int temp = strtoul(i, 0, 10);
		result.insert(temp);
		i = strchr(i, ',');
		if (i)
		{
			++i;
		}
	}
}
