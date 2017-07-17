#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "random.h"
#include "string.hpp"

#define BUFSZ 32768

std::string coda_strprintf(const char *fmt, ...)
{
	va_list ap;

	char *BUF = (char *) alloca(BUFSZ);
	va_start(ap, fmt);
	int bytes = vsnprintf(BUF, BUFSZ, fmt, ap);
	va_end(ap);

	if (BUFSZ <= bytes)
	{
		BUF = (char *) alloca(bytes + 1);
		va_start(ap, fmt);
		vsnprintf(BUF, bytes + 1, fmt, ap);
		va_end(ap);
	}

	return BUF;
}

int coda_strappend(std::string& out, const char* fmt, ...)
{
	va_list ap;

	char *BUF = (char *) alloca(BUFSZ);
	va_start(ap, fmt);
	int bytes = vsnprintf(BUF, BUFSZ, fmt, ap);
	va_end(ap);

	if (BUFSZ <= bytes)
	{
		BUF = (char *) alloca(bytes + 1);
		va_start(ap, fmt);
		vsnprintf(BUF, bytes + 1, fmt, ap);
		va_end(ap);
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

	va_end(ap);

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

	va_end(ap);

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
		sscanf(i, "%" PRIu64, &temp);
		result.insert(temp);
		i = strchr(i, ',');
		if (i)
		{
			++i;
		}
	}
}

std::vector<std::string> coda_split_string(const char *src, const char *delimiters, bool ignore_empty)
{
	std::vector<std::string> result;
	const char *prev_ptr = src;
	const char *p = src;

	for (; *p; ++p)
	{
		if (NULL != strchr(delimiters, *p))
		{
			if (prev_ptr < p || !ignore_empty)
			{
				result.push_back(std::string(prev_ptr, p - prev_ptr));
			}

			prev_ptr = p + 1;
		}
	}

	if (prev_ptr < p || !ignore_empty)
	{
		result.push_back(std::string(prev_ptr, p - prev_ptr));
	}

	return result;
}

std::vector<std::string> coda_split_string(const char *src, size_t size, const char *delimiters, bool ignore_empty)
{
	std::vector<std::string> result;
	const char *prev_ptr = src;
	const char *p = src;
	const char *e = src + size;

	for (; p < e; ++p)
	{
		if (NULL != strchr(delimiters, *p))
		{
			if (prev_ptr < p || !ignore_empty)
			{
				result.push_back(std::string(prev_ptr, p - prev_ptr));
			}

			prev_ptr = p + 1;
		}
	}

	if (prev_ptr < p || !ignore_empty)
	{
		result.push_back(std::string(prev_ptr, p - prev_ptr));
	}

	return result;
}

std::string coda_random_string(size_t size, const char *alphabet, size_t alphabet_size)
{
	coda_random_init();

	std::string result;

	for (; size--;)
	{
		unsigned idx = coda_random() % alphabet_size;
		result.push_back(alphabet[idx]);
	}

	return result;
}

