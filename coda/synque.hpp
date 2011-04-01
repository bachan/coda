#ifndef __CODA_SYNQUE_HPP__
#define __CODA_SYNQUE_HPP__

#include <pthread.h>
#include <deque>

namespace coda {

template <typename _T>
class synque
{
	pthread_cond_t  c;
	pthread_mutex_t m;
	std::deque<_T>  d;

public:
	synque()
	{
		pthread_mutex_init(&m, NULL);
		pthread_cond_init(&c, NULL);
	}

	~synque()
	{
		pthread_cond_destroy(&c);
		pthread_mutex_destroy(&m);
	}

	void add(const _T& e)
	{
		pthread_mutex_lock(&m);
		d.push_front(e);
		pthread_cond_signal(&c);
		pthread_mutex_unlock(&m);
	}

	void signal()
	{
		pthread_mutex_lock(&m);
		pthread_cond_signal(&c);
		pthread_mutex_unlock(&m);
	}

	int get(_T& e, int canwait)
	{
		pthread_mutex_lock(&m);

		if (d.empty())
		{
			if (0 == canwait)
			{
				pthread_mutex_unlock(&m);
				return -1;
			}

			pthread_cond_wait(&c, &m);

			if (d.empty())
			{
				pthread_mutex_unlock(&m);
				return -1;
			}
		}

		e = d.back();
		d.pop_back();

		pthread_mutex_unlock(&m);

		return 0;
	}

	size_t size()
	{
		pthread_mutex_lock(&m);
		size_t sz = d.size();
		pthread_mutex_unlock(&m);
		return sz;
	}
};

} /* namespace coda */

#endif /* __CODA_SYNQUE_HPP__ */
