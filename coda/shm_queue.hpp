#ifndef __SHM_QUEUE_H__
#define __SHM_QUEUE_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <coda/error.hpp>

#define DEFAULT_BOX_SIZE 8192
#define DEFAULT_MAX_BOXES_IN_QUEUE 1000

namespace shm_queue {

struct serializable
{
	serializable() {}
	virtual ~serializable() {}
	virtual bool serialize(void *p) const = 0;
	virtual bool deserialize(const void *p) = 0;
	virtual size_t size() const { return 0; }
};

struct queue_info
{
	enum { registers_number = 12 };

	uint32_t size;
	uint32_t box_size;
	uint32_t start;
	uint32_t end;
	uint32_t reserved[12];
}__attribute((packed));

template <typename _T>
class queue
{
	class box
	{
		uint8_t *ptr;
		size_t size;
		size_t pos;

	public:
		box();
		~box();

		void allocate(size_t sz);
		void deallocate();
		void clear();
		void copy_from(void *p);
		void copy_to(void *p) const;
		bool get(_T &h);
		bool put(const _T &h);
		bool is_space_for(const _T &h);
	};

	box current_box;
	queue_info *info;
	uint8_t *boxes;

	int sem_id;
	int shm_id;

	uint32_t BOX_SIZE;
	uint32_t MAX_BOXES_IN_QUEUE;

	void lock_queue();
	void unlock_queue();
	void reset_queue();

	void connect_shm(const char *key_fname);
	void connect_sem(const char *key_fname);

public:
	queue();
	~queue();

	void connect(const char *shm_key, const char *sem_key, uint32_t bs = DEFAULT_BOX_SIZE, uint32_t mbiq = DEFAULT_MAX_BOXES_IN_QUEUE);
	void disconnect();

	bool get(_T &h);
	bool unflush();

	bool put(const _T &h);
	void flush();

	void set_register(uint8_t offset, uint32_t var);
	uint32_t get_register(uint8_t offset);
	uint8_t registers_number()const;

	uint32_t pages_used() const;
	int semid() { return sem_id; }
	int shmid() { return shm_id; }
	int boxes_in_queue() const;
};

template <typename _T>
class reader : public queue<_T>
{
};

template <typename _T>
class writer : public queue<_T>
{
};

} /* namespace shm_queue */

#include "shm_queue.tcc"

#endif /* __SHM_QUEUE_HPP__ */
