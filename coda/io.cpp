#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "io.hpp"

int io::ostream::open(const std::string &fname)
{
	f = gzopen(fname.c_str(), "wb");
	if (f == 0)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "Error gz-open %s: (%d) %s", fname.c_str(), errno, strerror(errno));
		throw std::logic_error(errbuf);
	}
	fn = fname;
	return 0;
}

void io::ostream::close()
{
	gzclose(f);
	f = 0;
	fn.clear();
}

int io::istream::open(const std::string &fname)
{
	f = gzopen(fname.c_str(), "rb");
	if (f == 0)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "Error reading %s: (%d) %s", fname.c_str(), errno, strerror(errno));
		throw std::logic_error(errbuf);
	}
	fn = fname;
	return 0;
}

void io::istream::close()
{
	gzclose(f);
	f = 0;
	fn.clear();
}

