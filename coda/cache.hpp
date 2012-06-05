#ifndef __CODA_CACHE_HPP__
#define __CODA_CACHE_HPP__

#include <map>

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

		elem_t(typename data_t::iterator i_prev, typename data_t::iterator i_next)
			: prev(i_prev)
			, next(i_next)
		{
		}
	};

	data_t data;
	size_t size;

	typename data_t::iterator beg_it;
	typename data_t::iterator end_it;

public:
	coda_cache(size_t i_size)
		: size(i_size)
	{
		beg_it = data.end();
		end_it = data.end();
	}

	Val& get(const Key &key);
	void dbg();
};

#include "cache.tcc"

#endif /* __CODA_CACHE_HPP__ */
