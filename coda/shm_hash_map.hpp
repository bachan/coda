#ifndef __SHM_HASH_MAP_______
#define __SHM_HASH_MAP_______

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <stdexcept>

#include "shm_pool.hpp"

namespace shm {


template <typename _Type, int _OBJECTS_NUM, int _HASH_SIZE>
class hash_map
{
	int sem_id;
	int shm_id;

public:

	struct pair
	{
		uint64_t key;
		_Type value;

	};


	hash_map();
	~hash_map();

	//----------------------------------------------------------------------------------

	void connect(const char * shm_key, const char * sem_key) throw (std::exception);

	void disconnect();

	//----------------------------------------------------------------------------------

	int semid() {return sem_id;}
	int shmid() {return shm_id;}

	//----------------------------------------------------------------------------------

	size_t required_space()const;

	size_t size();

	size_t capacity();

	//----------------------------------------------------------------------------------

	bool read(uint64_t key, _Type& out);

	bool insert(uint64_t key, const _Type& val);
	bool insert(pair * pairs, size_t pairs_num);

	bool erase(uint64_t key);
	bool erase(uint64_t * keys, size_t keys_num);

	//----------------------------------------------------------------------------------

	void clear();

	bool is_connected();

protected:

	struct node
	{
		pair p;

		u_int32_t next;

	};

	u_int32_t * objects_num;
	u_int32_t * item_size;
	u_int32_t * hash_size;

	shm::pool<node, _OBJECTS_NUM> nodes_pool;

	u_int32_t * hash;

	u_int8_t * shmem_p;
 	node * objects;

	//----------------------------------------------------------------------------------

	void attach(void * mem_begin) throw(std::exception);
	void detach();

	void connect_shm(const char *key_fname);

	void connect_sem(const char *key_fname);

	//----------------------------------------------------------------------------------
public:
	bool unprotected_insert(uint64_t key, const _Type& val);
	bool unprotected_erase(uint64_t key);
	bool unprotected_read(uint64_t key, _Type& out);

	void lock_read_rw();
	void unlock_read_rw();

	void lock_write_rw();
	void unlock_write_rw();
};

}

#include "shm_hash_map.tcc"

#endif
