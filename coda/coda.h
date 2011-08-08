#ifndef __CODA_H__
#define __CODA_H__

#if __GNUC__ >= 3
#define CODA_INLINE      inline __attribute__ ((always_inline))
#define CODA_NOINLINE           __attribute__ ((noinline))
#define CODA_PURE               __attribute__ ((pure))
#define CODA_CONST              __attribute__ ((const))
#define CODA_NORETURN           __attribute__ ((noreturn))
#define CODA_MALLOC             __attribute__ ((malloc))
#define CODA_MUST_CHECK         __attribute__ ((warn_unused_result))
#define CODA_DEPRECATED         __attribute__ ((deprecated))
#define CODA_USED               __attribute__ ((used))
#define CODA_UNUSED             __attribute__ ((unused))
#define CODA_PACKED             __attribute__ ((packed))
#define CODA_ALIGN(x)           __attribute__ ((aligned (x)))
#define CODA_ALIGN_MAX          __attribute__ ((aligned))
#define CODA_FORMAT(n,f,e)	    __attribute__ ((format (n,f,e)))
#define CODA_LIKELY(x)          __builtin_expect (!!(x), 1)
#define CODA_UNLIKELY(x)        __builtin_expect (!!(x), 0)
#else
#define CODA_INLINE             /* no inline */
#define CODA_NOINLINE           /* no noinline */
#define CODA_PURE               /* no pure */
#define CODA_CONST              /* no const */
#define CODA_NORETURN           /* no noreturn */
#define CODA_MALLOC             /* no malloc */
#define CODA_MUST_CHECK         /* no warn_unused_result */
#ifdef _MSC_VER
#define CODA_DEPRECATED         __declspec(deprecated)
#else
#define CODA_DEPRECATED         /* no deprecated */
#endif
#define CODA_USED               /* no used */
#define CODA_UNUSED             /* no unused */
#define CODA_PACKED             /* no packed */
#define CODA_ALIGN(x)           /* no aligned (x) */
#define CODA_ALIGN_MAX          /* no aligned */
#define CODA_FORMAT(n,f,e)      /* no format */
#define CODA_LIKELY(x)          (x)
#define CODA_UNLIKELY(x)        (x)
#endif

#if 1 /* TODO discuss this one particular typedef, and more simple iovec analog with you know who */
typedef struct
{
	size_t size;
	char * data;
} strt, *strp;

/* #define coda_string(str) { sizeof(str) - 1, (uchp) str } */
/* #define coda_null_string { 0, NULL } */
#include <string.h>
#define coda_clrptr(p)   memset((p),0,sizeof(*(p)))
#define coda_clrvec(p,n) memset((p),0,sizeof(*(p))*(n))
#define coda_clrmem(p,n) memset((p),0,(n))
#endif

#define QUOTES_NAME(val) #val
#define QUOTES_DATA(val) QUOTES_NAME(val)
#define CONCAT_NAME(a,b) a##b
#define CONCAT_DATA(a,b) CONCAT_NAME(a,b)

#endif /* __CODA_H__ */
