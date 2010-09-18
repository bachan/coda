#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include "logger.h"

static
int fdmove(int fd, int nd) /* if nd is ebadf or eq-to-fd => fd is returned */
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

static
int fdopen(int fd, const char* path, int flags)
{
    int td;

    td = open(path, flags, 0644);
    if (0 > td) return -1;

    return fdmove(td, fd);
}

static
int mkpath(char* path)
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

volatile int log_lvcurr = LOG_INFO;

int log_lvread(const char* lv)
{
    switch (lv[0])
    {
    case 'a': case 'A':
        switch (lv[1])
        {
        case 'c': case 'C': return LOG_ACCESS;
        case 'l': case 'L': return LOG_ALERT;
        }
        break;
    case 'c': case 'C': return LOG_CRIT;
    case 'e': case 'E':
        switch (lv[1])
        {
        case 'm': case 'M': return LOG_EMERG;
        case 'r': case 'R': return LOG_ERROR;
        }
        break;
    case 'i': case 'I': return LOG_INFO;
    case 'n': case 'N': return LOG_NOTICE;
    case 'w': case 'W': return LOG_WARN;
    }

    return LOG_INFO; /* default */
}

int log_create(const char* fn, int lv)
{
    if (0 > mkpath((char *) fn)) return -1;
    if (0 > fdopen(STDERR_FILENO, fn, O_CREAT|O_APPEND|O_WRONLY))
        return -1;

    log_lvcurr = lv;

    return STDERR_FILENO;
}

