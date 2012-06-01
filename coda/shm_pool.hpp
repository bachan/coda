#ifndef __SHM_POOL_HPP__
#define __SHM_POOL_HPP__

#include <inttypes.h>
#include <coda/error.hpp>
#include "shm_stack.hpp"

namespace shm {

template <typename _Type, int _OBJECTS_NUM>
class pool
{
protected:
	shm::stack<uint32_t, _OBJECTS_NUM> free_nodes;

	uint32_t *reserved;
	uint32_t *item_size;
	uint32_t *current;

	_Type *data;

public:
	pool();
	~pool();

	void attach(void *header_begin, void *stack_begin, void *objects_begin) throw (std::exception);
	void detach();

	uint32_t allocate();
	bool free(uint32_t elem);
	size_t header_size()const;
	size_t stack_size()const;
	size_t data_size()const;
	size_t allocated_objects();
	void clear();
};

}

#include "shm_pool.tcc"

#endif /* __SHM_POOL_HPP__ */
