#ifndef __CODA_CACHE_HPP__
#define __CODA_CACHE_HPP__

#include <time.h>
#include <unordered_map>
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
	typedef std::unordered_map<Key, elem_t> data_t;
	typedef std::pair<const Key, elem_t> data_value_t; /* HACK: here we "know" about data_t::value_type */

	struct elem_t
	{
		Val elem;
		data_value_t *prev;
		data_value_t *next;

		size_t size;
		size_t time; /* time when element was created */

		elem_t()
			: prev(NULL)
			, next(NULL)
			, size(0)
		{
			time = ::time(NULL);
		}
	};

	data_t data;
	data_value_t *begp;
	data_value_t *endp;

	size_t size_cur;
	size_t size_max;
	size_t time_max;

	void drop_last_unused();

public:
	coda_cache(size_t i_size_max, size_t i_time_max)
		: begp(NULL)
		, endp(NULL)
		, size_cur(0)
		, size_max(i_size_max)
		, time_max(i_time_max)
	{
	}

	/* NOTE, that both get() functions could possibly erase outdated element */

	const Val *get(const Key &key);
	void get(const Key &key, Val &val);
	void set(const Key &key, const Val &val, bool do_update_time);

	bool erase(typename data_t::iterator it);
	bool erase(const Key &key);

	void dbg(std::string &res, int dbg) const;
};

#include "cache.tcc"

#endif /* __CODA_CACHE_HPP__ */
