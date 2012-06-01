#ifndef __SHM_STACK_HPP__
#define __SHM_STACK_HPP__

#include <inttypes.h>
#include <coda/error.hpp>

namespace shm {

template <typename _Type, int _OBJECTS_NUM>
class stack
{
protected:
	uint32_t *reserved;
	uint32_t *item_size;
	uint32_t *current;

	_Type *data;

public:
	stack();
	~stack();

	void attach(void * header_begin, void * block_begin) throw (std::exception);
	void detach();

	size_t header_size()const;
	size_t data_size()const;
	size_t used()const;

	bool empty()const;
	bool push(const _Type& el);
	_Type pop(void);
	void clear();
};

}

#include "shm_stack.tcc"

#endif /* __SHM_STACK_HPP__ */
