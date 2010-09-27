#ifndef __CODA_ERROR_HPP__
#define __CODA_ERROR_HPP__

#include <stdio.h>
#include <stdarg.h>
#include <exception>
#include "coda.h"
#include "error.h"

#define coda_errno(err,fmt,...) coda_error(fmt " (%d: %s)", \
	##__VA_ARGS__, err, coda_strerror(err))

class coda_error : public std::exception
{
	char msg [CODA_ERROR_BUFSZ];

public:
	coda_error(const char* fmt, ...) throw() CODA_FORMAT(printf, 2, 3)
	{
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(msg, CODA_ERROR_BUFSZ, fmt, ap);
	}

	virtual ~coda_error() throw() {}
	virtual const char* what() const throw() { return msg; }
	/* TODO: Implement self-logging for exceptions and related stuff */
	/* virtual int	self() const throw() { return log_emerg("exception: %s", msg); } */
};

#endif /* __CODA_ERROR_HPP__ */
