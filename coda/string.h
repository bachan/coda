#ifndef __CODA_STRING_H__
#define __CODA_STRING_H__

#include <ctype.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

char *coda_stristr(const char *s, const char *n);              /* search for [n] in [s] ignoring case */
char *coda_strnstr(const char *s, const char *n, size_t len);  /* search for [n] in first [len] bytes of [s] */
char *coda_strxstr(const char *s, const char *n, size_t len);  /* search for [n] in first [len] bytes of [s] ignoring case */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __CODA_STRING_H__ */
