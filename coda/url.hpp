#ifndef __CODA_URL_HPP__
#define __CODA_URL_HPP__

#include <sys/types.h>
#include <string>
#include <map>

typedef std::map<std::string, std::string> coda_params_t;
typedef std::map<std::string, std::string> coda_header_t;

int coda_get_params_buf(coda_params_t& params, const char* s, size_t len);
int coda_get_header_buf(coda_header_t& header, const char* s, size_t len);

int coda_get_params(coda_params_t& params, const char* s);
int coda_get_header(coda_header_t& header, const char* s);

void coda_url_unescape(const char* s, char* dest, size_t sz);
void coda_url_escape(const char* s, char* dest, size_t sz);
void coda_url_preunescape(const char* s, char* dest, size_t sz);


#endif /* __CODA_URL_HPP__ */
