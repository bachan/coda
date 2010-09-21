#ifndef __RDEV_H__
#define __RDEV_H__

#include "rdev_cattr.h"

/* Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */

#define makeq_(n...) #n
#define sizeq_(n...) sizeof(#n)-1
#define MAKEQ_(n...) makeq_(n)
#define SIZEQ_(n...) sizeof(makeq_(n))-1

#define typec_(name) typedef struct name (  name##_t )
#define fptrc_(name) typedef struct name (* name##_fp)

#define TYPEC_(name,type) typedef type (  name##_t )
#define FPTRC_(name,type) typedef type (* name##_fp)

#define ERFMT_(fmt) __FILE__":"MAKEQ_(__LINE__)": "fmt" in function "__func__"\n"
#define ERMSG_(fmt,...) fprintf(stderr,ERFMT_(fmt),##__VA_ARGS__)

#endif /* __RDEV_H__ */
