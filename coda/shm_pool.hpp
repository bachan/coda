#ifndef __SHM_POOL_______
#define __SHM_POOL_______

//------------------------------------------------------------------------------------------------
#include <sys/types.h>
#include <stdexcept>

#include "shm_stack.hpp"

namespace shm {
//------------------------------------------------------------------------------------------------

template <typename _Type, int _OBJECTS_NUM>
class pool
{
protected:

	shm::stack<u_int32_t, _OBJECTS_NUM> free_nodes;

	u_int32_t * reserved;
	u_int32_t * item_size;
	u_int32_t * current;

	_Type * data;

public:

	pool();
	~pool();

	void attach(void * header_begin, void * stack_begin, void * objects_begin) throw (std::exception);

	void detach();

	u_int32_t allocate();

	bool free(u_int32_t elem);

	size_t header_size()const;

	size_t stack_size()const;

	size_t data_size()const;

	size_t allocated_objects();

	void clear();
};
//------------------------------------------------------------------------------------------------
}

#include "shm_pool.tcc"
#endif
