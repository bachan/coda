#ifndef __CODA_ARRAYS_H__
#define __CODA_ARRAYS_H__

#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

char* coda_strnchr(const char* s, char c, size_t len);         /* search for [c] in first [len] bytes of [s] */
char* coda_strnstr(const char* s, const char* n, size_t len);  /* search for [n] in first [len] bytes of [s] */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __CODA_ARRAYS_H__ */
