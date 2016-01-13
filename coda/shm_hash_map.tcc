#include <errno.h>

#define mySEM_R 0
#define mySEM_W 1

inline uint32_t floor1024(uint32_t param)
{
	return (param + 1023) & ~(1023);
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::hash_map()
	: sem_id(-1)
	, shm_id(-1)
	, objects_num(0)
	, item_size(0)
	, hash_size(0)
	, hash(0)
	, shmem_p(0)
	, objects(0)
{
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::~hash_map()
{
	disconnect();
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
void shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::attach(void * mem_begin) throw(std::exception)
{
	uint8_t *mem_p = (uint8_t *) mem_begin;

	objects_num = (uint32_t *) mem_begin;
	item_size = objects_num + 1;
	hash_size = objects_num + 2;

	// some memory usage arrangement

	uint8_t *nodes_header_p = mem_p + 3 * sizeof(uint32_t);
	int map_header_size = floor1024(3 * sizeof(uint32_t) + nodes_pool.header_size());

	uint8_t *nodes_stack_p = mem_p + map_header_size;
	int map_stacks_size = floor1024(nodes_pool.stack_size());

	uint8_t *nodes_pool_p = mem_p + map_header_size + map_stacks_size;
	objects = (node *) nodes_pool_p;

	int map_data_size = floor1024(nodes_pool.data_size());
	uint8_t *hash_p = mem_p + map_header_size + map_stacks_size + map_data_size;

	if (0 == *objects_num && 0 == *item_size && 0 == *hash_size)
	{
		// kto perviy vstal togo i tapki!
		*objects_num = _OBJECTS_NUM;
		*item_size = sizeof(_Type);
		*hash_size = _HASH_SIZE;

		for (uint32_t i = 0; i < _HASH_SIZE; i++)
		{
			((uint32_t *) hash_p)[i] = (uint32_t)(-1);
		}
	}
	else if (_OBJECTS_NUM != *objects_num || sizeof(_Type) != *item_size || _HASH_SIZE != *hash_size)
	{
		throw coda_error("hash_map attaching error: expected hash_map<sizeof(_Type)=%d, %d, %d>, but target is hash_map<sizeof(_Type)=%d, %d, %d>",
			(int)sizeof(_Type), (int)_OBJECTS_NUM, (int)_HASH_SIZE, (int)*item_size, (int)*objects_num, (int)*hash_size);
	}

	nodes_pool.attach(nodes_header_p, nodes_stack_p, nodes_pool_p);
	hash = (uint32_t *) hash_p;

	shmem_p = mem_p;

	//printf("attach map: res %d, sz %d, hash %d\n", (int)*objects_num, (int)*item_size, *hash_size);
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
void shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::detach()
{
	objects_num = 0;
	item_size = 0;
	hash_size = 0;
	hash = 0;
	objects = 0;
	shmem_p = 0;

	nodes_pool.detach();
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
size_t shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::required_space()const
{
	int map_header_size = floor1024(3 * sizeof(uint32_t) + nodes_pool.header_size());
	int map_stacks_size = floor1024(nodes_pool.stack_size());
	int map_data_size = floor1024(nodes_pool.data_size());

	return map_header_size + map_stacks_size + map_data_size + sizeof(uint32_t) * _HASH_SIZE;
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
size_t shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::size()
{
	lock_read_rw();
	size_t ret = nodes_pool.allocated_objects();
	unlock_read_rw();
	return ret;
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
size_t shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::capacity()
{
	return _OBJECTS_NUM;
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
bool shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::unprotected_read(uint64_t key, _Type& out)
{
	int hash_index = key % _HASH_SIZE;
	uint32_t elem = hash[hash_index];

	while ((uint32_t) -1 != elem)
	{
		if (key == objects[elem].p.key)
		{
			out = objects[elem].p.value;
			return true;
		}
		else
		{
			elem = objects[elem].next;
		}
	}

	return false;
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
bool shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::read(uint64_t key, _Type& out)
{
	lock_read_rw();
	bool res = unprotected_read(key, out);
	unlock_read_rw();
	return res;
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
bool shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::insert(uint64_t key, const _Type& val)
{
	lock_write_rw();
	bool res = unprotected_insert(key, val);
	unlock_write_rw();
	return res;
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
bool shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::insert(pair * pairs, size_t pairs_num)
{
	lock_write_rw();

	size_t res = 0;

	for (size_t i = 0; i < pairs_num; i++)
	{
		res += unprotected_insert(pairs[i].key, pairs[i].value) ? 1 : 0;
	}

	unlock_write_rw();

	return res == pairs_num;
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
bool shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::erase(uint64_t key)
{
	lock_write_rw();
	bool res = unprotected_erase(key);
	unlock_write_rw();
	return res;
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
bool shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::erase(uint64_t * keys, size_t keys_num)
{
	lock_write_rw();

	size_t res = 0;

	for (size_t i = 0; i < keys_num; i++)
	{
		res += unprotected_erase(keys[i]) ? 1 : 0;
	}

	unlock_write_rw();

	return res == keys_num;
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
void shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::clear()
{
	lock_write_rw();

	nodes_pool.clear();

	for (uint32_t i = 0; i < _HASH_SIZE; i++)
	{
		hash[i] = (uint32_t) -1;
	}

	unlock_write_rw();
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
bool shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::is_connected()
{
	return shmem_p != 0;
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
bool shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::unprotected_insert(uint64_t key, const _Type& val)
{
	int hash_index = key % _HASH_SIZE;
	uint32_t el = hash[hash_index];

	if ((uint32_t) -1 != el)
	{
		while (true)
		{
			if (key == objects[el].p.key)
			{
				objects[el].p.value = val;
				return true;
			}

			if ((uint32_t) -1 != objects[el].next)
			{
				el = objects[el].next;
			}
			else
			{
				uint32_t new_node = nodes_pool.allocate();

				if ((uint32_t) -1 == new_node)
				{
					return false;
				}

				objects[new_node].p.key = key;
				objects[new_node].p.value = val;
				objects[new_node].next = (uint32_t)(-1);

				objects[el].next = new_node;

				return true;
			}
		}
	}
	else
	{
		uint32_t new_node = nodes_pool.allocate();

		if ((uint32_t) -1 == new_node)
		{
			return false;
		}

		objects[new_node].p.key = key;
		objects[new_node].p.value = val;
		objects[new_node].next = (uint32_t)(-1);

		hash[hash_index] = new_node;

		return true;
	}
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
bool shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::unprotected_erase(uint64_t key)
{
	int hash_index = key % _HASH_SIZE;
	uint32_t elem = hash[hash_index];

	if ((uint32_t) -1 != elem)
	{
		if (key == objects[elem].p.key)
		{
			hash[hash_index] = objects[elem].next;
			objects[elem].next = (uint32_t) -1;

			nodes_pool.free(elem);

			return true;
		}
		else
		{
			uint32_t prev = elem;

			elem = objects[elem].next;

			while ((uint32_t) -1 != elem)
			{
				if (key == objects[elem].p.key)
				{
					objects[prev].next = objects[elem].next;
					objects[elem].next = (uint32_t) -1;

					nodes_pool.free(elem);

					return true;
				}

				prev = elem;
				elem = objects[elem].next;
			}

			return false;
		}
	}
	else
	{
		return false;
	}
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
void shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::connect_shm(const char *key_fname)
{
	lock_write_rw();

	if (is_connected())
	{
		unlock_write_rw();
		throw coda_error("shm::hash_map is already connected");
	}

	key_t key;

	if ((key_t) -1 == (key = ftok(key_fname, 1)))
	{
		unlock_write_rw();
		throw coda_error("error creating key from file %s: %s", key_fname, coda_strerror(errno));
	}

	bool created = false;

	if (-1 == (shm_id = shmget(key, required_space(), 0666)))
	{
		// not created? creating...
		if (-1 == (shm_id = shmget(key, required_space(), IPC_CREAT | 0666)))
		{
			unlock_write_rw();
			throw coda_error("error creating queue shm: %s", coda_strerror(errno));
		}
		created = true;
	}

	//printf("ShmID: %d\n", shm_id);

	uint8_t *ptr = (uint8_t *) shmat(shm_id, 0, 0);

	if ((void*) -1 == ptr)
	{
		unlock_write_rw();
		throw coda_error("error attaching queue shmem: %s", coda_strerror(errno));
	}

	if (true == created)
	{
		memset(ptr, 0, 1024); //we clear only header memory space
	}

	shmem_p = ptr;

	unlock_write_rw();
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
void shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::connect_sem(const char *key_fname)
{
	if (-1 != sem_id)
	{
		throw coda_error("<hash_map> sem already connected");
	}

	key_t key;

	if ((key_t) -1 == (key = ftok(key_fname, 1)))
	{
		throw coda_error("error creating key from file %s: %s", key_fname, coda_strerror(errno));
	}

	if (-1 == (sem_id = semget(key, 2, 0)))
	{
		if (-1 == (sem_id = semget(key, 2, IPC_CREAT | 0666)))
		{
			throw coda_error("error getting sem: %s", coda_strerror(errno));
		}

		// now set initial values for _GUARD semaphores
		if (-1 == semctl(sem_id, mySEM_R, SETVAL, (0)))
		{
			throw coda_error("error setting sem: %s", coda_strerror(errno));
		}

		if (-1 == semctl(sem_id, mySEM_W, SETVAL, (0)))
		{
			throw coda_error("error setting sem: %s", coda_strerror(errno));
		}
	}
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
void shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::connect(const char *key_fname) throw (std::exception)
{
	try
	{
		connect_sem(key_fname);
		connect_shm(key_fname);
	}
	catch (const std::exception& e)
	{
		disconnect();
		throw;
	}

	try
	{
		lock_write_rw();
		attach(shmem_p);
		unlock_write_rw();
	}
	catch (const std::exception& e)
	{
		unlock_write_rw();
		disconnect();
		throw;
	}
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
void shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::disconnect()
{
	if (!is_connected())
	{
		return;
	}

	detach();

	sem_id = -1;
	shm_id = -1;

	shmdt(shmem_p);
	shmem_p = 0;
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
inline void shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::lock_read_rw()
{
	struct sembuf sem[2];
	sem[0].sem_num = mySEM_W;
	sem[0].sem_op  = 0;
	sem[0].sem_flg = SEM_UNDO;
	sem[1].sem_num = mySEM_R;
	sem[1].sem_op  = 1;
	sem[1].sem_flg = SEM_UNDO;

	if (0 > semop(sem_id, sem, 2))
	{
		throw coda_error("lock_read_rw : error '%s'", coda_strerror(errno));
	}
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
inline void shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::unlock_read_rw()
{
	struct sembuf sem;
	sem.sem_num = mySEM_R;
	sem.sem_op  = -1;
	sem.sem_flg = SEM_UNDO;

	if (0 > semop(sem_id, &sem, 1))
	{
		throw coda_error("lock_read_rw : error '%s'", coda_strerror(errno));
	}
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
inline void shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::lock_write_rw()
{
	struct sembuf sem[3];
	sem[0].sem_num = mySEM_R;
	sem[0].sem_op  = 0;
	sem[0].sem_flg = SEM_UNDO;
	sem[1].sem_num = mySEM_W;
	sem[1].sem_op  = 0;
	sem[1].sem_flg = SEM_UNDO;
	sem[2].sem_num = mySEM_W;
	sem[2].sem_op  = 1;
	sem[2].sem_flg = SEM_UNDO;

	if (0 > semop(sem_id, sem, 3))
	{
		throw coda_error("lock_read_rw : error '%s'", coda_strerror(errno));
	}
}

template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
inline void shm::hash_map<_Type, _OBJECTS_NUM, _HASH_SIZE>::unlock_write_rw()
{
	struct sembuf sem;
	sem.sem_num = mySEM_W;
	sem.sem_op  = -1;
	sem.sem_flg = SEM_UNDO;

	if (0 > semop(sem_id, &sem, 1))
	{
		throw coda_error("lock_read_rw : error '%s'", coda_strerror(errno));
	}
}

