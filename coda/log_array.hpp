#ifndef __TGADPFNRUVVRD__AUX_LOG_ARRAY_HPP__
#define __TGADPFNRUVVRD__AUX_LOG_ARRAY_HPP__

#include <sys/types.h>
#include <stdint.h>

namespace coda {

template <typename _T>
struct log_array_value_holder
{
	_T value;
	log_array_value_holder();
};

template<typename _T>
inline log_array_value_holder<_T>::log_array_value_holder()
{
}

template <>
inline log_array_value_holder<char>::log_array_value_holder()
	: value(0)
{
}

template <>
inline log_array_value_holder<unsigned char>::log_array_value_holder()
	: value(0)
{
}

template <>
inline log_array_value_holder<unsigned short int>::log_array_value_holder()
	: value(0)
{
}

template <>
inline log_array_value_holder<short int>::log_array_value_holder()
	: value(0)
{
}

template <>
inline log_array_value_holder<int>::log_array_value_holder()
	: value(0)
{
}

template <>
inline log_array_value_holder<unsigned int>::log_array_value_holder()
	: value(0)
{
}

template <>
inline log_array_value_holder<long int>::log_array_value_holder()
	: value(0)
{
}

template <>
inline log_array_value_holder<unsigned long int>::log_array_value_holder()
	: value(0)
{
}

template <>
inline log_array_value_holder<double>::log_array_value_holder()
	: value(0)
{
}

template <>
inline log_array_value_holder<float>::log_array_value_holder()
	: value(0)
{
}

template <typename _T, unsigned int MaxLog = 8>
class log_array
{
public:
	enum { max_log_value = MaxLog };
	typedef _T value_type;
private:
	log_array_value_holder<_T> data[MaxLog];
public:
	log_array()
	{
	}
	_T &get_by_value(size_t i);
	_T &get_at(size_t i)
	{
		return data[i].value;
	}
	const _T &get_at(size_t i) const
	{
		return data[i].value;
	}
	void swap(log_array<_T, MaxLog> &a2)
	{
		_T tmp;
		unsigned int i;
		for (i = 0; i < max_log_value; i++)
		{
			tmp = data[i].value;
			data[i].value = a2.data[i].value;
			a2.data[i].value = tmp;
		}
	}
};

template <typename _T, unsigned int MaxLog>
_T &log_array<_T, MaxLog>::get_by_value(size_t i)
{
	size_t j = (size_t)(-1);
	while (i)
	{
		i >>= 1;
		j++;
	}
	if (j < max_log_value) return get_at(j);
	return get_at(max_log_value - 1);
}

} // end namespace aux

#endif // __TGADPFNRUVVRD__AUX_LOG_ARRAY_HPP__

