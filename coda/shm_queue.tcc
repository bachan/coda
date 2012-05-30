#define SEM_ID_0 0
#define SEM_ID_QUEUE_COUNT 1

template <typename _T>
shm_queue::queue<_T>::box::box()
{
	ptr = NULL;
	pos = 0;
	size  = 0;
}

template <typename _T>
shm_queue::queue<_T>::box::~box()
{
	if (ptr)
	{
		deallocate();
	}
}

template <typename _T>
void shm_queue::queue<_T>::box::allocate(size_t sz)
{
	ptr = new uint8_t [sz];
	memset(ptr, 0, sz);
	size = sz;
	pos = 0;
}

template <typename _T>
void shm_queue::queue<_T>::box::deallocate()
{
	if (ptr)
	{
		delete [] ptr;

		ptr = 0;
		size = 0;
		pos = 0;
	}
}

template <typename _T>
void shm_queue::queue<_T>::box::copy_from(void * p)
{
	memcpy(ptr, p, size);
	pos = 0;
}

template <typename _T>
void shm_queue::queue<_T>::box::copy_to(void * p) const
{
	memcpy(p, ptr, size);
}

template <typename _T>
void shm_queue::queue<_T>::box::clear()
{
	memset(ptr, 0, size);
	pos = 0;
}

template <typename _T>
bool shm_queue::queue<_T>::box::get(_T& h)
{
	if (h.deserialize(ptr + pos))
	{
		pos += h.size();
		return true;
	}
	else
	{
		return false;
	}
}

template <typename _T>
bool shm_queue::queue<_T>::box::put(const _T & h)
{
	size_t sz = h.size();

	if (is_space_for(h) && h.serialize(ptr + pos))
	{
		pos += sz;
		return true;
	}
	else
	{
		return false;
	}
}

template <typename _T>
bool shm_queue::queue<_T>::box::is_space_for(const _T &h)
{
	return (pos + h.size() <= size);
}

template <typename _T>
shm_queue::queue<_T>::queue()
	: info(0)
	, boxes(0)
	, sem_id(-1)
	, shm_id(-1)
	, BOX_SIZE(0)
	, MAX_BOXES_IN_QUEUE(0)
{
}

template <typename _T>
shm_queue::queue<_T>::~queue()
{
	if (boxes)
	{
		disconnect();
	}
}

template <typename _T>
void shm_queue::queue<_T>::connect_shm(const char *key_fname)
{
	if (boxes)
	{
		throw coda_error("shm_queue::queue<>: queue already connected");
	}

	key_t key;

	if ((key_t) -1 == (key = ftok(key_fname, 1)))
	{
		throw coda_error("shm_queue::queue<>: error creating key from file '%s': %s", key_fname, coda_strerror(errno));
	}

	int created = 0;

	if (-1 == (shm_id = shmget(key, MAX_BOXES_IN_QUEUE * BOX_SIZE + sizeof(queue_info), 0666)))
	{
		if (-1 == (shm_id = shmget(key, MAX_BOXES_IN_QUEUE * BOX_SIZE + sizeof(queue_info), IPC_CREAT | 0666))) // not created? creating...
		{
			throw coda_error("shm_queue::queue<>: error creating queue shm: %s", coda_strerror(errno));
		}

		created = 1;
	}

	uint8_t *ptr = (uint8_t *) shmat(shm_id, 0, 0);

	if ((void*) -1 == ptr)
	{
		throw coda_error("shm_queue::queue<>: error attaching queue shmem: %s", coda_strerror(errno));
	}

	info = (shm_queue::queue_info *) ptr;

	if (created)
	{
		memset(ptr, 0, MAX_BOXES_IN_QUEUE * BOX_SIZE + sizeof(queue_info));
		info->box_size = BOX_SIZE;
		info->size = MAX_BOXES_IN_QUEUE;
	}

	ptr += sizeof(shm_queue::queue_info);

	if ((BOX_SIZE != info->box_size) || (MAX_BOXES_IN_QUEUE != info->size))
	{
		shmdt(info);
		info = 0;

		throw coda_error("box_size(%d) in queue is not equal to %d", info->box_size, BOX_SIZE);
	}

	boxes = ptr;
	current_box.allocate(BOX_SIZE);
}

template <typename _T>
void shm_queue::queue<_T>::reset_queue()
{
	if (info)
	{
		memset(info, 0, MAX_BOXES_IN_QUEUE * BOX_SIZE + sizeof(queue_info));
		info->box_size = BOX_SIZE;
		info->size = MAX_BOXES_IN_QUEUE;
	}
}

template <typename _T>
void shm_queue::queue<_T>::connect_sem(const char *key_fname)
{
	if (-1 != sem_id)
	{
		throw coda_error("<shm_queue::queue> sem already connected");
	}

	key_t key;

	if ((key_t) -1 == (key = ftok(key_fname, 1)))
	{
		throw coda_error("shm_queue::queue<>: error creating key from file '%s': %s", key_fname, coda_strerror(errno));
	}

	if (-1 == (sem_id = semget(key, 2, 0)))
	{
		if (-1 == (sem_id = semget(key, 2, IPC_CREAT | 0666)))
		{
			throw coda_error("shm_queue::queue<>:error getting sem: %s", coda_strerror(errno));
		}

		if (-1 == semctl(sem_id, SEM_ID_0, SETVAL, (1)))
		{
			throw coda_error("shm_queue::queue<>:error setting sem: %s", coda_strerror(errno));
		}

		if (-1 == semctl(sem_id, SEM_ID_QUEUE_COUNT, SETVAL, (0)))
		{
			throw coda_error("shm_queue::queue<>:error setting sem: %s", coda_strerror(errno));
		}
	}
}

template <typename _T>
void shm_queue::queue<_T>::connect(const char *key_fname, uint32_t bs, uint32_t mbiq)
{
	BOX_SIZE = bs;
	MAX_BOXES_IN_QUEUE = mbiq;

	connect_sem(key_fname);
	lock_queue();
	connect_shm(key_fname);
	unlock_queue();
}

template <typename _T>
void shm_queue::queue<_T>::disconnect()
{
	if (NULL == info)
	{
		return;
	}

	shmdt(info);
	info = NULL;
	boxes = NULL;
	current_box.deallocate();
	sem_id = -1;
	shm_id = -1;
	BOX_SIZE = 0;
	MAX_BOXES_IN_QUEUE = 0;
}

template <typename _T>
bool shm_queue::queue<_T>::get(_T &h)
{
	if (current_box.is_space_for(h))
	{
		return current_box.get(h);
	}
	else
	{
		return false;
	}
}

template <typename _T>
bool shm_queue::queue<_T>::unflush()
{
	if (boxes)
	{
		struct sembuf sem;
		sem.sem_num = SEM_ID_QUEUE_COUNT;
		sem.sem_op = -1;
		sem.sem_flg = 0;

		if (-1 == semop(sem_id, &sem, 1))
		{
			return false;
		}

		lock_queue();

		uint8_t *ptr = boxes + info->start * info->box_size;
		current_box.copy_from(ptr);
		memset(ptr, 0, info->box_size);

		info->start++;
		info->start %= info->size;

		unlock_queue();

		return true;
	}

	return false;
}

template <typename _T>
bool shm_queue::queue<_T>::put(const _T &h)
{
	if (current_box.put(h))
	{
		return true;
	}

	flush();
	return current_box.put(h);
}

template <typename _T>
void shm_queue::queue<_T>::flush()
{
	if (boxes)
	{
		lock_queue();
		if ((info->end + 1) % info->size != info->start)
		{
			uint8_t *ptr = boxes + info->end * info->box_size;
			current_box.copy_to(ptr);
			info->end++;
			info->end %= info->size;

			struct sembuf sem;
			sem.sem_num = SEM_ID_QUEUE_COUNT;
			sem.sem_op = 1;
			sem.sem_flg = 0;
			semop(sem_id, &sem, 1);
		}
		else
		{
			// no space to write... :/
		}
		unlock_queue();
	}
	current_box.clear();
}

template <typename _T>
int shm_queue::queue<_T>::boxes_in_queue()const
{
	return semctl(sem_id, SEM_ID_QUEUE_COUNT, GETVAL);
}

template <typename _T>
void shm_queue::queue<_T>::set_register(uint8_t offset, uint32_t var)
{
	if (offset < queue_info::registers_number)
	{
		lock_queue();
		info->reserved[offset] = var;
		unlock_queue();
	}
}

template <typename _T>
uint32_t shm_queue::queue<_T>::get_register(uint8_t offset)
{
	uint32_t var = 0;

	if (offset < queue_info::registers_number)
	{
		lock_queue();
		var = info->reserved[offset];
		unlock_queue();
	}

	return var;
}

template <typename _T>
uint8_t shm_queue::queue<_T>::registers_number()const
{
	return queue_info::registers_number;
}

template <typename _T>
uint32_t shm_queue::queue<_T>::pages_used()const
{
	return semctl(sem_id, SEM_ID_QUEUE_COUNT, GETVAL, (0));
}

template <typename _T>
void shm_queue::queue<_T>::lock_queue()
{
	struct sembuf sem;
	sem.sem_num = SEM_ID_0;
	sem.sem_op = -1;
	sem.sem_flg = SEM_UNDO;
	semop(sem_id, &sem, 1);
}

template <typename _T>
void shm_queue::queue<_T>::unlock_queue()
{
	struct sembuf sem;
	sem.sem_num = SEM_ID_0;
	sem.sem_op = 1;
	sem.sem_flg = SEM_UNDO;
	semop(sem_id, &sem, 1);
}

