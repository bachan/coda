#include <alloca.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "rdev_stat.h"

int rdev_pmkdir_nz(char *pathname, size_t pathsize, mode_t mode)
{
    int rc;
    char *p = pathname + 1;
    char *e = pathname + pathsize;

    if (!pathname) return EFAULT;

    for (; p < e; ++p)
    {
        if ('/' != *p) continue;

        *p = 0; rc = mkdir(pathname, mode);
        *p = '/';

        if (0 != rc && EEXIST != errno)
        {
            return errno;
        }
    }

    return 0;
}

int rdev_pmkdir(char *pathname, mode_t mode)
{
    int rc;
    char *p = pathname + 1;

    if (! pathname) return EFAULT;
    if (!*pathname) return ENOENT;

    for (; *p; ++p)
    {
        if ('/' != *p) continue;

        *p = 0; rc = mkdir(pathname, mode);
        *p = '/';

        if (0 != rc && EEXIST != errno)
        {
            return errno;
        }
    }

    return 0;
}

int rdev_pmkdir_nz_const(const char *pathname, size_t pathsize, mode_t mode)
{
    char* sPathname = (char*)alloca(pathsize);
    memcpy(sPathname, pathname, pathsize);
    return rdev_pmkdir_nz(sPathname, pathsize, mode);
}

int rdev_pmkdir_const(const char *pathname, mode_t mode)
{
    const size_t pathsize = strlen(pathname);
    return rdev_pmkdir_nz_const(pathname, pathsize, mode);
}

/* rdev_mapped_t read */

static rdev_mapped_t mcfg_readfn = { O_RDONLY,
    0, PROT_READ, MAP_PRIVATE, 0 };

int rdev_mapped_readfn(const char *fn, void *my_data, rdev_mapped_fp my_cb)
{
    return rdev_mapped_custom(fn, my_data, my_cb, &mcfg_readfn);
}

int rdev_mapped_custom(const char *fn, void *my_data, rdev_mapped_fp my_cb,
    const rdev_mapped_t *mcfg)
{
    int fd;
    struct stat st;
    void *mem;

    if (-1 == (fd = open(fn, mcfg->o_flag)))
    {
        return -1;
    }

    if (0 != fstat(fd, &st))
    {
        close(fd); return -1;
    }

    if (MAP_FAILED == (mem = mmap(NULL, st.st_size, mcfg->m_prot,
        mcfg->m_flag, fd, mcfg->ofst)))
    {
        close(fd); return -1;
    }

    if (0 != my_cb(my_data, mem, st.st_size))
    {
        munmap(mem, st.st_size);
        close(fd); return -1;
    }

    if (0 != munmap(mem, st.st_size))
    {
        close(fd); return -1;
    }

    if (0 != close(fd))
    {
        return -1;
    }

    return 0;
}

