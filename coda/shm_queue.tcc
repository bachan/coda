namespace shm_queue {

	enum T
	{
		BOX_SIZE = 8192,
		//BOX_SIZE = 200,
		MAX_BOXES_IN_QUEUE = 8192,

  		SEM_ID_0 = 0,
		SEM_ID_QUEUE_COUNT = 1
	};

} /* namespace shm_queue */

template <typename _T>
shm_queue::reader<_T>::box::box()
{
	ptr = NULL;
	r_pos = 0;
	size  = 0;
}

template <typename _T>
shm_queue::reader<_T>::box::~box()
{
	if(ptr)
	{
		deallocate();
	}
}

template <typename _T>
void shm_queue::reader<_T>::box::allocate(size_t sz)
{
	ptr = new u_int8_t[sz];
	memset(ptr, 0, sz);
	size = sz;
	r_pos = 0;
}

template <typename _T>
void shm_queue::reader<_T>::box::deallocate()
{
	if (0 != ptr)
	{
		delete[] ptr;

		ptr = 0;
		size = 0;
		r_pos = 0;
	}
}

template <typename _T>
void shm_queue::reader<_T>::box::copy_from(void * p)
{
	memcpy (ptr, p, size);
	r_pos = 0;
}

template <typename _T>
void shm_queue::reader<_T>::box::clear()
{
	memset (ptr, 0, size);
	r_pos = 0;
}

template <typename _T>
bool shm_queue::reader<_T>::box::get(_T& h)
{
	if(h.deserialize(ptr + r_pos))
	{
		r_pos += h.size();

		return true;
	}
	else
	{
		return false;
	}
}
template <typename _T>
bool shm_queue::reader<_T>::box::is_space_for(const _T &h)
{
	return (r_pos + h.size() <= size);
}
//------------------------------------------------------------------------
template <typename _T>
shm_queue::writer<_T>::box::box()
{
	ptr = NULL;
	w_pos = 0;
	size  = 0;
}

template <typename _T>
shm_queue::writer<_T>::box::~box ()
{
	if (ptr)
	{
		deallocate();
	}
}

template <typename _T>
void shm_queue::writer<_T>::box::allocate(size_t sz)
{
	ptr = new u_int8_t[sz];
	memset(ptr, 0, sz);
	size = sz;
	w_pos = 0;
}

template <typename _T>
void shm_queue::writer<_T>::box::deallocate()
{
	if (0 != ptr)
	{
		delete[] ptr;

		ptr = 0;
		size = 0;
		w_pos = 0;
	}
}

template <typename _T>
void shm_queue::writer<_T>::box::clear()
{
	memset (ptr, 0, size);
	w_pos = 0;
}

template <typename _T>
void shm_queue::writer<_T>::box::copy_to(void * p) const
{
	memcpy (p, ptr, size);
}

template <typename _T>
bool shm_queue::writer<_T>::box::put(const _T & h)
{
	size_t sz = h.size();

 	//printf("box::put\n   w_pos:%d h.size():%d box.size: %d\n", w_pos, sz, size);

	if(is_space_for(h) && h.serialize(ptr + w_pos))
	{
		w_pos += sz;
		//printf("/box::put  TRUE\n");
		return true;
	}
	else
	{
		//printf("/box::put  false\n");
		return false;
	}
}
template <typename _T>
bool shm_queue::writer<_T>::box::is_space_for(const _T & h)
{
	return (w_pos + h.size() <= size);
}
//------------------------------------------------------------------------
template <typename _T>
shm_queue::reader<_T>::reader() : info(0), boxes(0), sem_id(-1), shm_id(-1)
{
}
template <typename _T>
shm_queue::reader<_T>::~reader()
{
	if(boxes)
	{
		disconnect_shm();
	}
}
template <typename _T>
void shm_queue::reader<_T>::connect_shm(const char *key_fname)
{
	if(boxes)
	{
		throw std::logic_error("shm_queue::reader<>: queue already connected");
	}

	key_t key;
	key = ftok(key_fname, 1);
	if((key_t)(-1) == key)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "shm_queue::reader<>: error creating key from file '%s': %s", key_fname, strerror(errno));

		throw std::logic_error(errbuf);
	}

	int created = 0;
	shm_id = shmget(key, MAX_BOXES_IN_QUEUE * BOX_SIZE + sizeof(queue_info), 0666);
	if(-1 == shm_id)
	{
		// not created? creating...
		shm_id = shmget(key, MAX_BOXES_IN_QUEUE * BOX_SIZE + sizeof(queue_info), IPC_CREAT | 0666);
		if (-1 == shm_id)
		{
			char errbuf[1024];
			snprintf(errbuf, 1024, "shm_queue::reader<>: error creating queue shm: %s", strerror(errno));
			throw std::logic_error(errbuf);
		}
		created = 1;
	}

	u_int8_t * ptr = (u_int8_t *)shmat(shm_id, 0, 0);
	if((void*)(-1) == ptr)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "shm_queue::reader<>: error attaching queue shmem: %s", strerror(errno));

		throw std::logic_error(errbuf);
	}

	info = (shm_queue::queue_info*)ptr;
	if(created)
	{
		memset(ptr, 0, MAX_BOXES_IN_QUEUE * BOX_SIZE + sizeof(queue_info));
		info->box_size = BOX_SIZE;
		info->size = MAX_BOXES_IN_QUEUE;
	}

	ptr += sizeof(shm_queue::queue_info);
	if((BOX_SIZE != info->box_size) || (MAX_BOXES_IN_QUEUE != info->size))
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "box_size(%d) in queue is not equal to %d", info->box_size, BOX_SIZE);
		shmdt(info);
		info = 0;

		throw std::logic_error(errbuf);
	}
	boxes = ptr;

	current_read_box.allocate(BOX_SIZE);
}

template <typename _T>
void shm_queue::reader<_T>::reset_queue()
{
	if(info)
	{
		memset(info, 0, MAX_BOXES_IN_QUEUE * BOX_SIZE + sizeof(queue_info));
		info->box_size = BOX_SIZE;
		info->size = MAX_BOXES_IN_QUEUE;
	}
}

template <typename _T>
void shm_queue::reader<_T>::connect_sem(const char *key_fname)
{
	if(-1 != sem_id)
	{
		throw std::logic_error("<shm_queue::reader> sem already connected");
	}

	key_t key;
	key = ftok(key_fname, 1);
	if((key_t)(-1) == key)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "shm_queue::reader<>: error creating key from file '%s': %s", key_fname, strerror(errno));

		throw std::logic_error(errbuf);
	}

	sem_id = semget(key, 2, 0);
	if(-1 == sem_id)
	{
		sem_id = semget(key, 2, IPC_CREAT | 0666);
		if(-1 == sem_id)
		{
			char errbuf[1024];
			snprintf(errbuf, 1024, "shm_queue::reader<>:error getting sem: %s", strerror(errno));

			throw std::logic_error(errbuf);
		}
		if(-1 == semctl(sem_id, SEM_ID_0, SETVAL, (1)))
		{
			char errbuf[1024];
			snprintf(errbuf, 1024, "shm_queue::reader<>:error setting sem: %s", strerror(errno));

			throw std::logic_error(errbuf);
		}
		if(semctl(sem_id, SEM_ID_QUEUE_COUNT, SETVAL, (0)) == -1)
		{
			char errbuf[1024];
			snprintf(errbuf, 1024, "shm_queue::reader<>:error setting sem: %s", strerror(errno));
			throw std::logic_error(errbuf);
		}
	}
}

template <typename _T>
void shm_queue::reader<_T>::disconnect_shm()
{
	if(!info)
		return;

	shmdt(info);
	sem_id = -1;
	shm_id = -1;
	info = 0;
	boxes = 0;
	current_read_box.deallocate();
}

template <typename _T>
bool shm_queue::reader<_T>::get(_T &h)
{
	if(current_read_box.is_space_for(h))
	{
		return current_read_box.get(h);
	}
	else
	{
		return false;
	}
}

template <typename _T>
bool shm_queue::reader<_T>::unflush()
{
	if(boxes)
	{
		struct sembuf sem;
		sem.sem_num = SEM_ID_QUEUE_COUNT;
		sem.sem_op = -1;
		sem.sem_flg = 0;

		if(-1 == semop(sem_id, &sem, 1))
		{
			return false;
		}


		lock_queue();

		u_int8_t *ptr = boxes + info->start * info->box_size;
		current_read_box.copy_from(ptr);
		memset(ptr, 0, info->box_size);

		info->start++;
		info->start %= info->size;

		unlock_queue();

		return true;
	}

	return false;
}

template <typename _T>
int shm_queue::reader<_T>::boxes_in_queue()const
{
	return semctl(sem_id, SEM_ID_QUEUE_COUNT, GETVAL);
}

template <typename _T>
void shm_queue::reader<_T>::set_register(u_int8_t offset, u_int32_t var)
{
	if(offset <  queue_info::registers_number)
	{
		lock_queue();

		info->reserved[offset] = var;

		unlock_queue();
	}
}

template <typename _T>
u_int32_t shm_queue::reader<_T>::get_register(u_int8_t offset)
{
	u_int32_t var = 0;

	if(offset <  queue_info::registers_number)
	{
		lock_queue();

		var = info->reserved[offset];

		unlock_queue();
	}

	return var;
}

template <typename _T>
u_int8_t shm_queue::reader<_T>::registers_number()const
{
	return queue_info::registers_number;
}

template <typename _T>
u_int32_t shm_queue::reader<_T>::pages_used()const
{
	return semctl(sem_id, SEM_ID_QUEUE_COUNT, GETVAL, (0));
}

template <typename _T>
void shm_queue::reader<_T>::lock_queue()
{
	struct sembuf sem;
	sem.sem_num = SEM_ID_0;
	sem.sem_op = -1;
	sem.sem_flg = SEM_UNDO;

	semop(sem_id, &sem, 1);
}

template <typename _T>
void shm_queue::reader<_T>::unlock_queue()
{
	struct sembuf sem;
	sem.sem_num = SEM_ID_0;
	sem.sem_op = 1;
	sem.sem_flg = SEM_UNDO;

	semop(sem_id, &sem, 1);
}

//------------------------------------------------------------------------------------------------
template <typename _T>
shm_queue::writer<_T>::writer() : info(0), boxes(0), sem_id(-1), shm_id(-1)
{
}

template <typename _T>
shm_queue::writer<_T>::~writer()
{
	if(boxes)
	{
		disconnect_shm();
	}
}

template <typename _T>
void shm_queue::writer<_T>::connect_shm(const char *key_fname)
{
	if(boxes)
	{
		throw std::logic_error("shm_queue::writer<>: queue already connected");
	}

	key_t key;
	key = ftok(key_fname, 1);
	if((key_t)(-1) == key)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "shm_queue::writer<>: error creating key from file '%s': %s", key_fname, strerror(errno));

		throw std::logic_error(errbuf);
	}

	int created = 0;
	shm_id = shmget(key, MAX_BOXES_IN_QUEUE * BOX_SIZE + sizeof(queue_info), 0666);
	if(-1 == shm_id)
	{
		// not created? creating...
		shm_id = shmget(key, MAX_BOXES_IN_QUEUE * BOX_SIZE + sizeof(queue_info), IPC_CREAT | 0666);
		if (-1 == shm_id)
		{
			char errbuf[1024];
			snprintf(errbuf, 1024, "shm_queue::writer<>: error creating queue shm: %s", strerror(errno));

			throw std::logic_error(errbuf);
		}
		created = 1;
	}

	u_int8_t *ptr = (u_int8_t *)shmat(shm_id, 0, 0);
	if((void*)(-1) == ptr)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "shm_queue::writer<>: error attaching queue shmem: %s", strerror(errno));

		throw std::logic_error(errbuf);
	}

	info = (shm_queue::queue_info*)ptr;
	if(created)
	{
		memset(ptr, 0, MAX_BOXES_IN_QUEUE * BOX_SIZE + sizeof(queue_info));
		info->box_size = BOX_SIZE;
		info->size = MAX_BOXES_IN_QUEUE;
	}

	ptr += sizeof(shm_queue::queue_info);
	if((BOX_SIZE != info->box_size) || (MAX_BOXES_IN_QUEUE != info->size))
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "shm_queue::writer<>: box_size(%d) in queue is not equal to %d", info->box_size, BOX_SIZE);
		shmdt(info);
		info = 0;

		throw std::logic_error(errbuf);
	}
	boxes = ptr;

	current_write_box.allocate(BOX_SIZE);
}

template <typename _T>
void shm_queue::writer<_T>::reset_queue()
{
	if(info)
	{
		memset(info, 0, MAX_BOXES_IN_QUEUE * BOX_SIZE + sizeof(queue_info));
		info->box_size = BOX_SIZE;
		info->size = MAX_BOXES_IN_QUEUE;
	}
}

template <typename _T>
void shm_queue::writer<_T>::connect_sem(const char *key_fname)
{
	if(-1 != sem_id)
	{
		throw std::logic_error("shm_queue::writer<>: sem already connected");
	}

	key_t key;
	key = ftok(key_fname, 1);
	if((key_t)(-1) == key)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "shm_queue::writer<>: error creating key from file '%s': %s", key_fname, strerror(errno));

		throw std::logic_error(errbuf);
	}

	sem_id = semget(key, 2, 0);
	if(sem_id == -1)
	{
		sem_id = semget(key, 2, IPC_CREAT | 0666);
		if(sem_id == -1)
		{
			char errbuf[1024];
			snprintf(errbuf, 1024, "shm_queue::writer<>: error getting sem: %s", strerror(errno));
			throw std::logic_error(errbuf);
		}

		if(semctl(sem_id, SEM_ID_0, SETVAL, (1)) == -1)
		{
			char errbuf[1024];
			snprintf(errbuf, 1024, "shm_queue::writer<>: error setting sem: %s", strerror(errno));
			throw std::logic_error(errbuf);
		}
		if(semctl(sem_id, SEM_ID_QUEUE_COUNT, SETVAL, (0)) == -1)
		{
			char errbuf[1024];
			snprintf(errbuf, 1024, "shm_queue::writer<>: error setting sem: %s", strerror(errno));
			throw std::logic_error(errbuf);
		}
	}
}

template <typename _T>
void shm_queue::writer<_T>::disconnect_shm()
{
	if(!info)
		return;

	shmdt(info);

	info = 0;
	boxes = 0;
	current_write_box.deallocate();
	sem_id = -1;
	shm_id = -1;
}

template <typename _T>
bool shm_queue::writer<_T>::put(const _T &h)
{
	if(current_write_box.put(h))
	{
		return true;
	}

	flush();
	return current_write_box.put(h);
}

template <typename _T>
void shm_queue::writer<_T>::flush()
{
	if(boxes)
	{
		lock_queue();
		if((info->end + 1) % info->size != info->start)
		{
			u_int8_t *ptr = boxes + info->end * info->box_size;
			current_write_box.copy_to(ptr);
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
	current_write_box.clear();
}

template <typename _T>
int shm_queue::writer<_T>::boxes_in_queue()const
{
	return semctl(sem_id, SEM_ID_QUEUE_COUNT, GETVAL);
}

template <typename _T>
void shm_queue::writer<_T>::set_register(u_int8_t offset, u_int32_t var)
{
	if(offset <  queue_info::registers_number)
	{
		lock_queue();

		info->reserved[offset] = var;

		unlock_queue();
	}
}

template <typename _T>
u_int32_t shm_queue::writer<_T>::get_register(u_int8_t offset)
{
	u_int32_t var = 0;

	if(offset <  queue_info::registers_number)
	{
		lock_queue();

		var = info->reserved[offset];

		unlock_queue();
	}

	return var;
}

template <typename _T>
u_int8_t shm_queue::writer<_T>::registers_number()const
{
	return queue_info::registers_number;
}

template <typename _T>
u_int32_t shm_queue::writer<_T>::pages_used()const
{
	return semctl(sem_id, SEM_ID_QUEUE_COUNT, GETVAL, (0));
}

template <typename _T>
void shm_queue::writer<_T>::lock_queue()
{
	struct sembuf sem;
	sem.sem_num = SEM_ID_0;
	sem.sem_op = -1;
	sem.sem_flg = SEM_UNDO;

	semop(sem_id, &sem, 1);
}

template <typename _T>
void shm_queue::writer<_T>::unlock_queue()
{
	struct sembuf sem;
	sem.sem_num = SEM_ID_0;
	sem.sem_op = 1;
	sem.sem_flg = SEM_UNDO;

	semop(sem_id, &sem, 1);
}


