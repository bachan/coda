#ifndef __CODA_CACHE_HPP__
#define __CODA_CACHE_HPP__

#include <time.h>
#include <map>
#include "estimate_capacity.hpp"
#include "string.hpp"

/*
 * Associated container, with maxiumum size of i_size_max bytes (oldest
 * elements are removed first when exceeded, TODO set to 0 for no size limit)
 * and maximum element lifetime of i_time_max (set to 0 for no limit)
 */

template <typename Key, typename Val>
class coda_cache
{
	struct elem_t;
	typedef std::map<Key, elem_t> data_t;

	struct elem_t
	{
		Val elem;

		typename data_t::iterator prev;
		typename data_t::iterator next;

		size_t size;
		size_t time; /* time when element was created */

		elem_t(typename data_t::iterator i_prev, typename data_t::iterator i_next)
			: prev(i_prev)
			, next(i_next)
			, size(0)
		{
			time = ::time(NULL);
		}
	};

	data_t data;
	size_t size_cur;
	size_t size_max;
	size_t time_max;

	typename data_t::iterator beg_it;
	typename data_t::iterator end_it;

	void drop_last_unused();

public:
	coda_cache(size_t i_size_max, size_t i_time_max)
		: size_cur(0)
		, size_max(i_size_max)
		, time_max(i_time_max)
	{
		beg_it = data.end();
		end_it = data.end();
	}

	const Val *get(const Key &key) const;

	void get_copy(const Key &key, Val &val);
	void set(const Key &key, const Val &val, bool do_update_time);

	bool erase(typename data_t::iterator it);
	bool erase(const Key &key);

	void dbg(std::string &res, int dbg) const;
};

#include "cache.tcc"

#endif /* __CODA_CACHE_HPP__ */
