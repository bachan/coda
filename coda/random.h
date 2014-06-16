#ifndef __CODA_RANDOM_H__
#define __CODA_RANDOM_H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

int coda_random_init();
#define coda_random() random()

#ifdef __cplusplus
}
#endif

#endif /* __CODA_RANDOM_H__ */
