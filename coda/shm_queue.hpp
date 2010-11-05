#ifndef __SHM_QUEUE_H__
#define __SHM_QUEUE_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdexcept>
#include <errno.h>

namespace shm_queue {

struct serializable
{
	serializable(){}
	virtual ~serializable(){}
	virtual bool serialize(void *p)const = 0;
	virtual bool deserialize(const void *p) = 0;

	virtual size_t size()const{return 0;}
};

struct queue_info
{
	enum {registers_number=12};

	u_int32_t size;
	u_int32_t box_size;
	u_int32_t start;
	u_int32_t end;
	u_int32_t reserved[12];
}__attribute((packed));

template <typename _T>
class reader
{
	class box
	{
		u_int8_t * ptr;
		size_t size;
		size_t r_pos;

	public:
		box();
		~box();

		void allocate(size_t sz);
		void deallocate();
		void clear();
		void copy_from(void *p);
		bool get(_T &h);
		bool is_space_for(const _T &h);
	}
	current_read_box;

	queue_info *info;
	u_int8_t *boxes;

	int sem_id;
	int shm_id;

	void lock_queue();
	void unlock_queue();
	void reset_queue();

public:
	reader();
	~reader();

	void connect_shm(const char *key_fname);
	void connect_sem(const char *key_fname);
	void disconnect_shm();

	bool get(_T &h);
	bool unflush();

	void set_register(u_int8_t offset, u_int32_t var);
	u_int32_t get_register(u_int8_t offset);
	u_int8_t registers_number()const;

	u_int32_t pages_used()const;
	int semid(){return sem_id;}
	int shmid(){return shm_id;}
	int boxes_in_queue()const;
};

template <typename _T>
class writer
{
	class box
	{
		u_int8_t * ptr;
		size_t size;
		size_t w_pos;

	public:
		box();
		~box();

		void allocate(size_t sz);
		void deallocate();
		void clear();
		void copy_to(void *p)const;
		bool put(const _T &h);
		bool is_space_for(const _T &h);
	}
	current_write_box;

	queue_info *info;
	u_int8_t *boxes;

	int sem_id;
	int shm_id;

	void lock_queue();
	void unlock_queue();
	void reset_queue();

public:
	writer();
	~writer();

	void connect_shm(const char *key_fname);
	void connect_sem(const char *key_fname);
	void disconnect_shm();

	bool put(const _T &h);

	void flush();

	void set_register(u_int8_t offset, u_int32_t var);
	u_int32_t get_register(u_int8_t offset);
	u_int8_t registers_number()const;

	u_int32_t pages_used()const;
	int semid(){return sem_id;}
	int shmid(){return shm_id;}
	int boxes_in_queue()const;
};

} /* namespace shm_queue */

#include "shm_queue.tcc"

#endif /* __SHM_QUEUE_HPP__ */

