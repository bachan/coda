#ifndef __RDEV_ERROR_H__
#define __RDEV_ERROR_H__

#include <string.h>
#include "rdev_cattr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RDEV_ERROR_BUFSZ 4096

#if (RDEV_USE_THREADS)
inline const char *rdev_strerror(int errnum)
{
    char errbuf [RDEV_ERROR_BUFSZ];

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
    if (0 != strerror_r(errnum, errbuf, RDEV_ERROR_BUFSZ)) return "XSI strerror_r returned error.";
    return errbuf;  /* will be read from stack */
#else
    return strerror_r(errnum, errbuf, RDEV_ERROR_BUFSZ);
#endif /* _POSIX_C_SOURCE >= ... */
}
#else
#define rdev_strerror(errnum) strerror(errnum)
#endif /* (RDEV_USE_THREADS) */

#if 0
inline const char *rdev_strerrno(int errnum)
{
    char errbuf [RDEV_ERROR_BUFSZ];
     int errlen = snprintf(errbuf, RDEV_ERROR_BUFSZ, "%d: ", errnum);

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
    if (0 != strerror_r(errnum, errbuf + errlen, RDEV_ERROR_BUFSZ - errlen)) return "XSI strerror_r returned error.";
    return errbuf;
#else
    strerror_r(errnum, errbuf + errlen, RDEV_ERROR_BUFSZ - errlen);
    return errbuf;
#endif /* _POSIX_C_SOURCE >= ... */
}
#endif

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __RDEV_ERROR_H__ */
