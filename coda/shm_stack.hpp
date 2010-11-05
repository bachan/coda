#ifndef __SHM_STACK_______
#define __SHM_STACK_______
//------------------------------------------------------------------------------------------------

#include <sys/types.h>
#include <stdexcept>

namespace shm {


template <typename _Type, int _OBJECTS_NUM>
class stack
{
protected:

	u_int32_t * reserved;
	u_int32_t * item_size;

	u_int32_t * current;

	_Type * data;

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
//------------------------------------------------------------------------------------------------
}
#include "shm_stack.tcc"

#endif
