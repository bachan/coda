#ifndef __WEB64_H__
#define __WEB64_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#define WEB64_LENGTH(inlen) ((((inlen) + 2) / 3) * 4)
#define WEB64_DECODE_LENGTH(outlen) ((((outlen) + 3) / 4) * 3)

extern void web64_encode(const char *in, size_t inlen, char *out, size_t outlen);
extern bool web64_decode(const char *in, size_t inlen, char *out, size_t *outlen);

#ifdef __cplusplus
}
#endif

#endif /* __WEB64_H__ */
