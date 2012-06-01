#ifndef __SHM_HASH_MAP_HPP__
#define __SHM_HASH_MAP_HPP__

#include <inttypes.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <coda/error.hpp>
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

	void connect(const char *key_fname) throw (std::exception);
	void disconnect();

	int semid() {return sem_id;}
	int shmid() {return shm_id;}

	size_t required_space()const;
	size_t size();
	size_t capacity();

	bool read(uint64_t key, _Type& out);
	bool insert(uint64_t key, const _Type& val);
	bool insert(pair * pairs, size_t pairs_num);
	bool erase(uint64_t key);
	bool erase(uint64_t * keys, size_t keys_num);

	void clear();
	bool is_connected();

protected:
	struct node
	{
		pair p;
		uint32_t next;
	};

	uint32_t *objects_num;
	uint32_t *item_size;
	uint32_t *hash_size;

	shm::pool<node, _OBJECTS_NUM> nodes_pool;

	uint32_t *hash;

	uint8_t *shmem_p;
 	node *objects;

	void attach(void * mem_begin) throw(std::exception);
	void detach();

	void connect_shm(const char *key_fname);
	void connect_sem(const char *key_fname);

public:
	bool unprotected_insert(uint64_t key, const _Type& val);
	bool unprotected_erase(uint64_t key);
	bool unprotected_read(uint64_t key, _Type& out);

	void lock_read_rw();
	void unlock_read_rw();

	void lock_write_rw();
	void unlock_write_rw();

	struct const_iterator
	{
		uint32_t* hash;
		uint32_t position_in_hash;
		node* pool;
		uint32_t position_in_pool;

		const_iterator(uint32_t* _hash, uint32_t _position_in_hash, node* _pool, uint32_t _position_in_pool)
			: hash(_hash)
			, position_in_hash(_position_in_hash)
			, pool(_pool)
			, position_in_pool(_position_in_pool)
		{
		}

		bool operator!= (const const_iterator& rhs)
		{
			return (position_in_hash != rhs.position_in_hash || position_in_pool != rhs.position_in_pool);
		}

		const_iterator& operator++()
		{
			if (pool[position_in_pool].next != (uint32_t)(-1))
			{
				position_in_pool = pool[position_in_pool].next;
			}
			else
			{
				uint32_t i;
				for (i = position_in_hash + 1; i < _HASH_SIZE; ++i)
				{
					if (hash[i] != (uint32_t)(-1))
					{
						position_in_hash = i;
						position_in_pool = hash[i];
						break;
					}
				}
				if (i == _HASH_SIZE)
				{
					position_in_hash = -1;
					position_in_pool = -1;
				}
			}

			return *this;
		}

		uint64_t key()
		{
			return pool[position_in_pool].p.key;
		}

		const _Type& value()
		{
			return pool[position_in_pool].p.value;
		}
	};

	const_iterator begin()
	{
		for (uint32_t i = 0; i < _HASH_SIZE; ++i)
		{
			uint32_t el = hash[i];
			if (el != (uint32_t)(-1))
			{
				return const_iterator(hash, i, objects, el);
			}
		}

		return const_iterator(hash, -1, objects, -1);
	}

	const_iterator end()
	{
		return const_iterator(hash, -1, objects, -1);
	}
};

}

#include "shm_hash_map.tcc"

#endif /* __SHM_HASH_MAP_HPP__ */
