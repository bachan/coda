template <typename _Type, int _OBJECTS_NUM>
shm::pool<_Type, _OBJECTS_NUM>::pool()
	: free_nodes()
	, reserved(0)
	, item_size(0)
	, data(0)
{
}

template <typename _Type, int _OBJECTS_NUM>
shm::pool<_Type, _OBJECTS_NUM>::~pool()
{
	detach();
}

template <typename _Type, int _OBJECTS_NUM>
void shm::pool<_Type, _OBJECTS_NUM>::attach(void *header_begin, void *stack_begin, void *objects_begin) throw (std::exception)
{
	reserved = (uint32_t *) header_begin;
	item_size = reserved + 1;
	current = reserved + 2;

	free_nodes.attach((uint8_t *) header_begin + 3 * sizeof(uint32_t), stack_begin); //here we can easily get an exception...

	data = (_Type *) objects_begin;

	if (0 == *reserved && 0 == *item_size)
	{
		//kto perviy vstal togo i tapki!
		*reserved = _OBJECTS_NUM;
		*item_size = sizeof(_Type);
		*current = 0;
	}
	else if (_OBJECTS_NUM != *reserved || sizeof(_Type) != *item_size)
	{
		throw coda_error("Pool attaching error: expected %d objects with sizeof %d , but target has %d objects with sizeof %d",
			(int)_OBJECTS_NUM, (int)sizeof(_Type), (int)*reserved, (int)*item_size);
	}

	//printf("attach pool: res %d, sz %d, current %d\n", (int)*reserved, (int)*item_size, *current);
}

template <typename _Type, int _OBJECTS_NUM>
void shm::pool<_Type, _OBJECTS_NUM>::detach()
{
	free_nodes.detach();

	reserved = 0;
	current = 0;
	data = 0;
}

template <typename _Type, int _OBJECTS_NUM>
uint32_t shm::pool<_Type, _OBJECTS_NUM>::allocate()
{
	if (free_nodes.empty())
	{
		return (*current >= _OBJECTS_NUM) ? -1 : (*current)++;
	}
	else
	{
		return free_nodes.pop();
	}
}

template <typename _Type, int _OBJECTS_NUM>
bool shm::pool<_Type, _OBJECTS_NUM>::free(uint32_t elem)
{
	return free_nodes.push(elem);
}

template <typename _Type, int _OBJECTS_NUM>
size_t shm::pool<_Type, _OBJECTS_NUM>::header_size()const
{
	return 3 * sizeof(uint32_t) + free_nodes.header_size();
}

template <typename _Type, int _OBJECTS_NUM>
size_t shm::pool<_Type, _OBJECTS_NUM>::stack_size()const
{
	return free_nodes.data_size();
}

template <typename _Type, int _OBJECTS_NUM>
size_t shm::pool<_Type, _OBJECTS_NUM>::data_size()const
{
	return sizeof(_Type) * _OBJECTS_NUM;
}

template <typename _Type, int _OBJECTS_NUM>
size_t shm::pool<_Type, _OBJECTS_NUM>::allocated_objects()
{
	return *current - free_nodes.used();
}

template <typename _Type, int _OBJECTS_NUM>
void shm::pool<_Type, _OBJECTS_NUM>::clear()
{
	*current = 0;
	free_nodes.clear();
}

