#ifndef __CODA_ESTIMATE_CAPACITY_HPP__
#define __CODA_ESTIMATE_CAPACITY_HPP__

#include <vector>
#include <string>

template<typename T> inline
size_t estimate_capacity(const T &v)
{
	return v.estimate_capacity();
}

/* specialization for any pointer */
template<typename T> inline
size_t estimate_capacity(T *v)
{
	return sizeof(v);
}

/* specialization for any vector */
template<typename T> inline
size_t estimate_capacity(const std::vector<T> &v)
{
	return sizeof(v) + v.capacity() * sizeof(T);
}

/* specialization for any string */
template<typename T> inline
size_t estimate_capacity(const std::basic_string<T> &v)
{
	return sizeof(v) + v.capacity() * sizeof(T);
}

template<> inline
size_t estimate_capacity(const char &v)
{
	return sizeof(v);
}

template<> inline
size_t estimate_capacity(const unsigned char &v)
{
	return sizeof(v);
}

template<> inline
size_t estimate_capacity(const short &v)
{
	return sizeof(v);
}

template<> inline
size_t estimate_capacity(const unsigned short &v)
{
	return sizeof(v);
}

template<> inline
size_t estimate_capacity(const int &v)
{
	return sizeof(v);
}

template<> inline
size_t estimate_capacity(const unsigned int &v)
{
	return sizeof(v);
}

template<> inline
size_t estimate_capacity(const long &v)
{
	return sizeof(v);
}

template<> inline
size_t estimate_capacity(const unsigned long &v)
{
	return sizeof(v);
}

template<> inline
size_t estimate_capacity(const long long &v)
{
	return sizeof(v);
}

template<> inline
size_t estimate_capacity(const unsigned long long &v)
{
	return sizeof(v);
}

template<> inline
size_t estimate_capacity(const float &v)
{
	return sizeof(v);
}

template<> inline
size_t estimate_capacity(const double &v)
{
	return sizeof(v);
}

template<> inline
size_t estimate_capacity(const long double &v)
{
	return sizeof(v);
}

#endif /* __CODA_ESTIMATE_CAPACITY_HPP__ */
