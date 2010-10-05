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

#define coda_realpath(path) realpath((const char*) path, alloca(PATH_MAX))

static inline
int coda_fdmove(int fd, int nd) /* if nd is ebadf or eq-to-fd => fd is returned */
{
	if (0 > nd || fd == nd)
	{
		return fd;
	}

	nd = dup2(fd, nd);
	if (0 > nd) return -1;

	if (0 > close(fd))
	{
		return -1;
	}

	return nd;
}

static inline
int coda_fdopen(int fd, const char* path, int flags)
{
	int td;

	td = open(path, flags, 0644);
	if (0 > td) return -1;

	return coda_fdmove(td, fd);
}

static inline
int coda_mkpath(char* path)
{
	int rc;
	char* p = path + 1;

	for (; *p; ++p)
	{
		if (*p != '/') continue;

		*p = 0;
		rc = mkdir(path, 0755);
		*p = '/';

		if (0 > rc && EEXIST != errno)
		{
			return -1;
		}
	}

	return 0;
}

static inline
int coda_mkpidf(const char* path)
{
	int fd;

	fd = open(path, O_CREAT|O_WRONLY|O_TRUNC, 0644);
	if (0 > fd) return -1;

	if (0 > dprintf(fd, "%d\n", (int) getpid()))
	{
		close(fd);
		return -1;
	}

	if (0 > close(fd))
	{
		return -1;
	}

	return 0;
}

/* 
 * In fact, open_flags should be defined by both prot_flags and mmap_flags values,
 * and the definition of this interdependence doesn't look trivial sometimes.
 *
 * However, my guess is that almost in all cases file descriptor must be readable,
 * e.g. MAP_SHARED + PROT_WRITE require open with O_RDWR (not O_WRONLY).
 *
 * See, mmap(2).
 */

#define coda_umap(area) munmap((area)->data, (area)->size)
#define coda_read_file(area,path) coda_mmap(coda_clrptr(area), PROT_READ, MAP_PRIVATE, path) /* TODO coda_clrptr */

static inline
int coda_mmap(strp area, int prot_flags, int mmap_flags, const char* filename)
{
    int fd;
    int open_flags;

    if (NULL == filename) /* MAP_ANONYMOUS */
    {
        area->data = mmap(NULL, area->size, prot_flags,
            mmap_flags|MAP_ANONYMOUS, -1, 0);

        if (MAP_FAILED == area->data) return -1;

        return 0;
    }

    open_flags = (prot_flags & PROT_WRITE)
        ? O_RDWR : O_RDONLY;

    fd = open(filename, open_flags);
    if (0 > fd) return -1;

    if (0 != area->size)
    {
        if (0 > ftruncate(fd, area->size))
        {
            close(fd);
            return -1;
        }
    }
    else
    {
        struct stat st;

        if (0 > fstat(fd, &st))
        {
            close(fd);
            return -1;
        }

        area->size = st.st_size;
    }

    area->data = mmap(NULL, area->size, prot_flags,
        mmap_flags, fd, 0);

    if (MAP_FAILED == area->data)
    {
        close(fd);
        return -1;
    }

    if (0 > close(fd))
    {
        return -1;
    }

    return 0;
}

#endif /* __CODA_SYSTEM_H__ */
