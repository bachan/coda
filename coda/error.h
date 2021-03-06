#ifndef __CODA_ERROR_H__
#define __CODA_ERROR_H__

#ifndef __FreeBSD__
#include <alloca.h>
#endif /* __FreeBSD__ */
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
#if !defined(__USE_GNU) || defined(__FreeBSD__)
    if (0 != strerror_r(err, data, size)) return "XSI strerror_r returned error";
    return data;
#else
    return strerror_r(err, data, size);
#endif /* __USE_GNU */
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __CODA_ERROR_H__ */
