#ifndef __CODA_BASE58_H__
#define __CODA_BASE58_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

extern bool coda_isbase58 (char ch);

extern void coda_base58_encode (const char * in, size_t inlen,
               char * out, size_t outlen);

extern size_t coda_base58_encode_alloc (const char *in, size_t inlen, char **out);

extern bool coda_base58_decode (const char * in, size_t inlen,
               char * out, size_t *outlen);

extern bool coda_base58_decode_alloc (const char *in, size_t inlen,
                 char **out, size_t *outlen);

#ifdef __cplusplus
}
#endif

#endif /* __CODA_BASE58_H__ */
