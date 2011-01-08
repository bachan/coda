#ifndef __CODA_STRING_HPP__
#define __CODA_STRING_HPP__

#include <string>
#include <coda/coda.h>

int coda_strappend (std::string &out, const char *fmt, ...) CODA_FORMAT(printf, 2, 3);
int coda_strnappend(std::string &out, size_t n, const char *fmt, ...) CODA_FORMAT(printf, 3, 4);
int coda_strnprintf(std::string &out, size_t pos, size_t n, const char *fmt, ...) CODA_FORMAT(printf, 4, 5);

#endif /* __CODA_STRING_HPP__ */
