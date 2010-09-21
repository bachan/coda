#ifndef __RDEV_CATTR_H__
#define __RDEV_CATTR_H__

#if __GNUC__ >= 3
#define RDEV_INLINE      inline __attribute__ ((always_inline))
#define RDEV_NOINLINE           __attribute__ ((noinline))
#define RDEV_PURE               __attribute__ ((pure))
#define RDEV_CONST              __attribute__ ((const))
#define RDEV_NORETURN           __attribute__ ((noreturn))
#define RDEV_MALLOC             __attribute__ ((malloc))
#define RDEV_MUST_CHECK         __attribute__ ((warn_unused_result))
#define RDEV_DEPRECATED         __attribute__ ((deprecated))
#define RDEV_USED               __attribute__ ((used))
#define RDEV_UNUSED             __attribute__ ((unused))
#define RDEV_PACKED             __attribute__ ((packed))
#define RDEV_ALIGN(x)           __attribute__ ((aligned (x)))
#define RDEV_ALIGN_MAX          __attribute__ ((aligned))
#define RDEV_LIKELY(x)          __builtin_expect (!!(x), 1)
#define RDEV_UNLIKELY(x)        __builtin_expect (!!(x), 0)
#define RDEV_FORMAT(n, f, e)    __attribute__ ((format (n, f, e)))
#else
#define RDEV_INLINE             /* no inline */
#define RDEV_NOINLINE           /* no noinline */
#define RDEV_PURE               /* no pure */
#define RDEV_CONST              /* no const */
#define RDEV_NORETURN           /* no noreturn */
#define RDEV_MALLOC             /* no malloc */
#define RDEV_MUST_CHECK         /* no warn_unused_result */
#ifdef _MSC_VER
#define RDEV_DEPRECATED         __declspec(deprecated)
#else
#define RDEV_DEPRECATED         /* no deprecated */
#endif
#define RDEV_USED               /* no used */
#define RDEV_UNUSED             /* no unused */
#define RDEV_PACKED             /* no packed */
#define RDEV_ALIGN(x)           /* no aligned (x) */
#define RDEV_ALIGN_MAX          /* no aligned */
#define RDEV_LIKELY(x)          (x)
#define RDEV_UNLIKELY(x)        (x)
#define RDEV_FORMAT(n, f, e)    /* no format */
#endif

#endif /* __RDEV_CATTR_H__ */
