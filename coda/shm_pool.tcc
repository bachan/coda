namespace shm {
//------------------------------------------------------------------------------------------------

template <typename _Type, int _OBJECTS_NUM>
pool<_Type, _OBJECTS_NUM>::pool() : free_nodes(), reserved(0), item_size(0), data(0)
{

}

template <typename _Type, int _OBJECTS_NUM>
pool<_Type, _OBJECTS_NUM>::~pool()
{
	detach();
}

template <typename _Type, int _OBJECTS_NUM>
void pool<_Type, _OBJECTS_NUM>::attach(void * header_begin, void * stack_begin, void * objects_begin) throw (std::exception)
{
	reserved = static_cast<u_int32_t*>(header_begin);
	item_size = reserved + 1;
	current = reserved + 2;

	free_nodes.attach(static_cast<u_int8_t*>(header_begin) + 3 * sizeof(u_int32_t), stack_begin); //here we can easily get an exception...

	data = static_cast<_Type*>(objects_begin);

	if(0 == *reserved && 0 == *item_size)
	{
		//kto perviy vstal togo i tapki!
		*reserved = _OBJECTS_NUM;
		*item_size = sizeof(_Type);
		*current = 0;
	}
	else if(_OBJECTS_NUM != *reserved || sizeof(_Type) != *item_size)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "Pool attaching error: expected %d objects with sizeof %d , but target has %d objects with sizeof %d", (int)_OBJECTS_NUM, (int)sizeof(_Type), (int)*reserved, (int)*item_size);
			throw std::logic_error(errbuf);
	}

	//printf("attach pool: res %d, sz %d, current %d\n", (int)*reserved, (int)*item_size, *current);
}

template <typename _Type, int _OBJECTS_NUM>
void pool<_Type, _OBJECTS_NUM>::detach()
{
	free_nodes.detach();

	reserved = 0;
	current = 0;
	data = 0;
}

template <typename _Type, int _OBJECTS_NUM>
u_int32_t pool<_Type, _OBJECTS_NUM>::allocate()
{
	if(free_nodes.empty())
	{
		return (*current >= _OBJECTS_NUM) ? -1 : (*current)++;
	}
	else
	{
		return free_nodes.pop();
	}
}

template <typename _Type, int _OBJECTS_NUM>
bool pool<_Type, _OBJECTS_NUM>::free(u_int32_t elem)
{
	return free_nodes.push(elem);
}

template <typename _Type, int _OBJECTS_NUM>
size_t pool<_Type, _OBJECTS_NUM>::header_size()const
{
	return 3 * sizeof(u_int32_t) + free_nodes.header_size();
}

template <typename _Type, int _OBJECTS_NUM>
size_t pool<_Type, _OBJECTS_NUM>::stack_size()const
{
	return free_nodes.data_size();
}

template <typename _Type, int _OBJECTS_NUM>
size_t pool<_Type, _OBJECTS_NUM>::data_size()const
{
	return sizeof(_Type) * _OBJECTS_NUM;
}

template <typename _Type, int _OBJECTS_NUM>
size_t pool<_Type, _OBJECTS_NUM>::allocated_objects()
{
	return *current - free_nodes.used();
}

template <typename _Type, int _OBJECTS_NUM>
void pool<_Type, _OBJECTS_NUM>::clear()
{
	*current = 0;
	free_nodes.clear();
}
//------------------------------------------------------------------------------------------------
}
