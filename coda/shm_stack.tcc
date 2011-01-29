#include <stdio.h>

namespace shm {
//------------------------------------------------------------------------------------------------

template <typename _Type, int _OBJECTS_NUM>
stack<_Type, _OBJECTS_NUM>::stack() : reserved(0), item_size(0), current(0), data(0){}

template <typename _Type, int _OBJECTS_NUM>
stack<_Type, _OBJECTS_NUM>::~stack()
{
	detach();
}

template <typename _Type, int _OBJECTS_NUM>
inline void stack<_Type, _OBJECTS_NUM>::attach(void * header_begin, void * block_begin) throw (std::exception)
{
	reserved = static_cast<u_int32_t*>(header_begin);
	item_size = reserved + 1;
	current = reserved + 2;

	data = static_cast< _Type* >(block_begin);

	if(0 == *reserved && 0 == *item_size)
	{
		//kto perviy vstal togo i tapki!
		*reserved = _OBJECTS_NUM;
		*item_size = sizeof(_Type);
		*current = (u_int32_t)(-1);
	}
	else if(_OBJECTS_NUM != *reserved || sizeof(_Type) != *item_size)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "Stack attaching error: expected %d objects with sizeof %d , but target has %d objects with sizeof %d", (int)_OBJECTS_NUM, (int)sizeof(_Type), (int)*reserved, (int)*item_size);

		throw std::logic_error(errbuf);
	}

	//printf("attach stack: res %d, sz %d, current %d\n", (int)*reserved, (int)*item_size, *current);
}

template <typename _Type, int _OBJECTS_NUM>
inline void stack<_Type, _OBJECTS_NUM>::detach()
{
	reserved = 0;
	current = 0;
	item_size = 0;
	data = 0;
}
template <typename _Type, int _OBJECTS_NUM>
inline size_t stack<_Type, _OBJECTS_NUM>::header_size()const
{
	return 3 * sizeof(u_int32_t);
}

template <typename _Type, int _OBJECTS_NUM>
inline size_t stack<_Type, _OBJECTS_NUM>::data_size()const
{
	return sizeof(_Type) * _OBJECTS_NUM;
}

template <typename _Type, int _OBJECTS_NUM>
inline size_t stack<_Type, _OBJECTS_NUM>::used()const
{
	return *current + 1;
}

template <typename _Type, int _OBJECTS_NUM>
inline bool stack<_Type, _OBJECTS_NUM>::empty()const
{
	return used() == 0;
}

template <typename _Type, int _OBJECTS_NUM>
inline bool stack<_Type, _OBJECTS_NUM>::push(const _Type& el)
{
	if(used() >= _OBJECTS_NUM)
	{
		return false;
	}
	else
	{
		data[++(*current)] = el;

		return true;
	}
}
template <typename _Type, int _OBJECTS_NUM>
inline _Type stack<_Type, _OBJECTS_NUM>::pop(void)
{
	return data[(*current)--];
}

template <typename _Type, int _OBJECTS_NUM>
inline void stack<_Type, _OBJECTS_NUM>::clear(void)
{
	*current = (u_int32_t)(-1);
}

//------------------------------------------------------------------------------------------------
}
