#ifndef __CODA_CACHE_LIMITED_BY_MEMORY_HPP__
#define __CODA_CACHE_LIMITED_BY_MEMORY_HPP__

#include <map>

template <typename Key, typename Val>
class coda_cache_memlim
{
	struct elem_t;
	typedef std::map<Key, elem_t> data_t;

	struct elem_t
	{
		Val elem;

		typename data_t::iterator prev;
		typename data_t::iterator next;

		size_t link; /* number of links to element */
		size_t size;

		elem_t(typename data_t::iterator i_prev, typename data_t::iterator i_next)
			: prev(i_prev)
			, next(i_next)
			, link(0)
			, size(0)
		{
		}
	};

	data_t data;
	size_t size;
	size_t size_cur;

	typename data_t::iterator beg_it;
	typename data_t::iterator end_it;

	void drop_last_unused();

public:
	coda_cache_memlim(size_t i_size)
		: size(i_size)
		, size_cur(0)
	{
		beg_it = data.end();
		end_it = data.end();
	}

	Val& acquire(const Key &key);
	void release(const Key &key);

	void dbg();

	size_t get_size() const
	{
		return size_cur;
	}
};

#include "cache_memlim.tcc"

#endif /* __CODA_CACHE_LIMITED_BY_MEMORY_HPP__ */
