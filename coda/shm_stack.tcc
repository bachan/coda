template <typename _Type, int _OBJECTS_NUM>
shm::stack<_Type, _OBJECTS_NUM>::stack()
	: reserved(0)
	, item_size(0)
	, current(0)
	, data(0)
{
}

template <typename _Type, int _OBJECTS_NUM>
shm::stack<_Type, _OBJECTS_NUM>::~stack()
{
	detach();
}

template <typename _Type, int _OBJECTS_NUM>
inline void shm::stack<_Type, _OBJECTS_NUM>::attach(void * header_begin, void * block_begin) throw (std::exception)
{
	reserved = (uint32_t *) header_begin;
	item_size = reserved + 1;
	current = reserved + 2;

	data = (_Type *) block_begin;

	if (0 == *reserved && 0 == *item_size)
	{
		//kto perviy vstal togo i tapki!
		*reserved = _OBJECTS_NUM;
		*item_size = sizeof(_Type);
		*current = (uint32_t)(-1);
	}
	else if (_OBJECTS_NUM != *reserved || sizeof(_Type) != *item_size)
	{
		throw coda_error("Stack attaching error: expected %d objects with sizeof %d , but target has %d objects with sizeof %d",
			(int)_OBJECTS_NUM, (int)sizeof(_Type), (int)*reserved, (int)*item_size);
	}

	//printf("attach stack: res %d, sz %d, current %d\n", (int)*reserved, (int)*item_size, *current);
}

template <typename _Type, int _OBJECTS_NUM>
inline void shm::stack<_Type, _OBJECTS_NUM>::detach()
{
	reserved = 0;
	current = 0;
	item_size = 0;
	data = 0;
}
template <typename _Type, int _OBJECTS_NUM>
inline size_t shm::stack<_Type, _OBJECTS_NUM>::header_size()const
{
	return 3 * sizeof(uint32_t);
}

template <typename _Type, int _OBJECTS_NUM>
inline size_t shm::stack<_Type, _OBJECTS_NUM>::data_size()const
{
	return sizeof(_Type) * _OBJECTS_NUM;
}

template <typename _Type, int _OBJECTS_NUM>
inline size_t shm::stack<_Type, _OBJECTS_NUM>::used()const
{
	return *current + 1;
}

template <typename _Type, int _OBJECTS_NUM>
inline bool shm::stack<_Type, _OBJECTS_NUM>::empty()const
{
	return used() == 0;
}

template <typename _Type, int _OBJECTS_NUM>
inline bool shm::stack<_Type, _OBJECTS_NUM>::push(const _Type& el)
{
	if (used() >= _OBJECTS_NUM)
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
inline _Type shm::stack<_Type, _OBJECTS_NUM>::pop(void)
{
	return data[(*current)--];
}

template <typename _Type, int _OBJECTS_NUM>
inline void shm::stack<_Type, _OBJECTS_NUM>::clear(void)
{
	*current = (uint32_t) -1;
}

