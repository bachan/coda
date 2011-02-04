#ifndef __CODA_IO_HPP__
#define __CODA_IO_HPP__

#include <errno.h>
#include <string.h>
#include <zlib.h>
#include <stdexcept>
#include <string>
#include <map>
#include <tr1/array>
#include <tr1/unordered_map>
#include <google/sparse_hash_map>
#include <google/sparse_hash_set>
#include "log_array.hpp"

// XXX WARNING!!!
// THIS CODE IS DEPRECATED! DO NOT USE IT IN NEW CODE
// RATED AS "TAK PISHUT PIDORASI" BY PIANIST

namespace io {

class ostream
{
	std::string fn;
	gzFile f;
public:
	ostream() : f(0) {}
	~ostream() throw()
	{
		if (f) gzclose(f);
	};
	int open(const std::string &fname);
	void close();
	void write(void *buf, size_t sz)
	{
		if (sz && (gzwrite(f, buf, sz) == 0))
		{
			char errbuf[1024];
			snprintf(errbuf, 1024, "Error gz-writing %u bytes to %s: (%d) %s", (unsigned)sz, fn.c_str(), errno, strerror(errno));
			throw std::logic_error(errbuf);
		}
	}
	const std::string &name() const
	{
		return fn;
	}
};

class istream
{
	std::string fn;
	gzFile f;
public:
	istream(): f(0) {}
	~istream() throw()
	{
		if (f) gzclose(f);
	};
	int open(const std::string &fname);
	void close();
	void read(void *buf, size_t sz)
	{
		if (sz && (gzread(f, buf, sz) == 0))
		{
			char errbuf[1024];
			snprintf(errbuf, 1024, "Error gz-reading %s: (%d) %s", fn.c_str(), errno, strerror(errno));
			throw std::logic_error(errbuf);
		}
	}
	const std::string &name() const
	{
		return fn;
	}
};

} // end namespace io

inline io::ostream& operator << (io::ostream& o, const uint8_t &v)
{
	o.write((void*)&v, sizeof(uint8_t));
	return o;
}

inline io::istream& operator >> (io::istream& i, uint8_t &v)
{
	i.read((void*)&v, sizeof(uint8_t));
	return i;
}

inline io::ostream& operator << (io::ostream& o, const uint16_t &v)
{
	o.write((void*)&v, sizeof(uint16_t));
	return o;
}

inline io::istream& operator >> (io::istream& i, uint16_t &v)
{
	i.read((void*)&v, sizeof(uint16_t));
	return i;
}

inline io::ostream& operator << (io::ostream& o, const uint32_t &v)
{
	o.write((void*)&v, sizeof(uint32_t));
	return o;
}

inline io::istream& operator >> (io::istream& i, uint32_t &v)
{
	i.read((void*)&v, sizeof(uint32_t));
	return i;
}

inline io::ostream& operator << (io::ostream& o, const uint64_t &v)
{
	o.write((void*)&v, sizeof(uint64_t));
	return o;
}

inline io::istream& operator >> (io::istream& i, uint64_t &v)
{
	i.read((void*)&v, sizeof(uint64_t));
	return i;
}

inline io::ostream& operator << (io::ostream& o, const std::string &s)
{
	size_t s_sz = s.size();
	uint8_t ws_sz = s_sz < 255 ? s_sz : 255;
	o << ws_sz;
	o.write((void*)s.c_str(), ws_sz);
	return o;
}

inline io::istream& operator >> (io::istream& i, std::string &s)
{
	uint8_t sz;
	i >> sz;
	char buf[256];
	i.read((void*)&buf, sz);
	buf[sz] = 0;
	s = buf;
	return i;
}

template <typename _T>
io::ostream& operator << (io::ostream& o, const std::map<std::string, _T>&);

template <typename _T>
io::istream& operator >> (io::istream& i, std::map<std::string, _T>&);

template <typename _T>
io::ostream& operator << (io::ostream& o, const std::map<uint32_t, _T>&);

template <typename _T>
io::istream& operator >> (io::istream& i, std::map<uint32_t, _T>&);

template <typename _T>
io::ostream& operator << (io::ostream& o, const std::tr1::unordered_map<uint32_t, _T>&);

template <typename _T>
io::istream& operator >> (io::istream& i, std::tr1::unordered_map<uint32_t, _T>&);

template <typename _T>
io::ostream& operator << (io::ostream& o, const std::tr1::unordered_map<uint32_t, _T>&);

template <typename _T>
io::istream& operator >> (io::istream& i, google::sparse_hash_map<uint32_t, _T>&);

template <typename _T>
io::ostream& operator << (io::ostream& o, const google::sparse_hash_map<uint64_t, _T>&);

template <typename _T>
io::istream& operator >> (io::istream& i, std::tr1::unordered_map<uint64_t, _T>&);

template <typename _T, unsigned int Max>
io::ostream& operator << (io::ostream& o, const coda::log_array<_T, Max> &a);

template <typename _T, unsigned int Max>
io::istream& operator >> (io::istream& i, coda::log_array<_T, Max> &a);

template <typename _T, unsigned Max>
void save_tr1_array(io::ostream& o, const std::tr1::array<_T, Max> &a);

template <typename _T, unsigned int Max>
void load_tr1_array(io::istream& i, std::tr1::array<_T, Max> &a);

#include "io.tcc"

#endif // __CODA_IO_HPP__
