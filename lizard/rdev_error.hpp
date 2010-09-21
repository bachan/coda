#ifndef __RDEV_ERROR_HPP__
#define __RDEV_ERROR_HPP__

#include <stdio.h>
#include <stdarg.h>
#include <exception>
#include "rdev.h"
#include "rdev_error.h"

class rdev_error : public std::exception
{
    char msg [RDEV_ERROR_BUFSZ];

public:
    rdev_error(const char *fmt, ...) throw() RDEV_FORMAT(printf, 2, 3)
    {
        va_list ap; va_start(ap, fmt);
        vsnprintf(msg, RDEV_ERROR_BUFSZ, fmt, ap);
    }

    rdev_error(int errnum, const char *s) throw()
    {
        snprintf(msg, RDEV_ERROR_BUFSZ, "%s: %d: %s",
            s, errnum, rdev_strerror(errnum));
    }

    virtual ~rdev_error() throw() {}
    virtual const char *what() const throw() { return msg; }
};

#endif /* __RDEV_ERROR_HPP__ */
