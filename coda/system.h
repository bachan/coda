#ifndef __CODA_SYSTEM_H__
#define __CODA_SYSTEM_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "coda.h"

#ifdef __cplusplus
extern "C" {
#endif

int coda_fdmove(int fd, int nd);
int coda_fdopen(int fd, const char* path, int flags);
int coda_mkpath(char* path);
int coda_mkpidf(const char* path);

int coda_mmap(strp area, int prot_flags, int mmap_flags, const char* filename);
#define coda_umap(area) munmap((area)->data, (area)->size)
#define coda_mmap_file(area,path) coda_mmap((strp) coda_clrptr(area), PROT_READ, MAP_PRIVATE, path)

#ifdef __cplusplus
}
#endif

#endif /* __CODA_SYSTEM_H__ */
