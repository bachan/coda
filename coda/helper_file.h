#ifndef __CODA_FILE_H__
#define __CODA_FILE_H__

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

#endif /* __CODA_FILE_H__ */
