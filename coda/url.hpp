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

/* 
 * encode [src] of size [sz_src] to [dst], write trailing '\0'
 * to [dst] and return count of bytes written to [dst] (without
 * trailing '\0').
 */
size_t coda_urlenc(u_char* dst, const u_char* src, size_t sz_src);

/* 
 * decode [src] of size [sz_src] to [dst], write trailing '\0'
 * to [dst] and return count of bytes written to [dst] (without
 * trailing '\0').
 */
size_t coda_urldec(u_char* dst, const u_char* src, size_t sz_src);

#endif /* __CODA_URL_HPP__ */
