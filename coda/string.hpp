#ifndef __CODA_STRING_HPP__
#define __CODA_STRING_HPP__

#include <coda/coda.h>
#include <inttypes.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <set>

std::string coda_strprintf(const char *fmt, ...) CODA_FORMAT(printf, 1, 2);

int coda_strappend (std::string& out, const char* fmt, ...) CODA_FORMAT(printf, 2, 3);
int coda_strnappend(std::string& out, size_t num, const char* fmt, ...) CODA_FORMAT(printf, 3, 4);
int coda_strnprintf(std::string& out, size_t pos, size_t num, const char* fmt, ...) CODA_FORMAT(printf, 4, 5);

void coda_get_stringset(const char* str, std::set<std::string>& result);
void coda_get_uint64set(const char* str, std::set<uint64_t>& result);

std::vector<std::string> coda_split_string(const char *src, const char *delimiters, bool ignore_empty = true);
std::vector<std::string> coda_split_string(const char *src, size_t size, const char *delimiters, bool ignore_empty = true);

std::string coda_random_string(size_t size, const char *alphabet, size_t alphabet_size);

#endif /* __CODA_STRING_HPP__ */
