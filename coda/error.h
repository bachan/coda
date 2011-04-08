#ifndef __CODA_ERROR_H__
#define __CODA_ERROR_H__

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CODA_ERROR_BUFSZ 4096
#define coda_strerror(err) coda_strerror_r(err, (char*) \
	alloca(CODA_ERROR_BUFSZ), CODA_ERROR_BUFSZ)

static inline
const char* coda_strerror_r(int err, char* data, size_t size)
{
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
    if (0 != strerror_r(err, data, size)) return "XSI strerror_r returned error";
    return data;  /* will be read from stack */
#else
    return strerror_r(err, data, size);
#endif /* _POSIX_C_SOURCE >= ... */
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __CODA_ERROR_H__ */
