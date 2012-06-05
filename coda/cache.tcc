#include <iostream>
#include "cache.hpp"

template <typename Key, typename Val>
Val& coda_cache<Key, Val>::get(const Key &key)
{
	std::pair<typename data_t::iterator, bool> res;

	res = data.insert(typename data_t::value_type(key, elem_t(data.end(), data.end())));

//	log_info("cache::get size=%d, max_sz=%d", (int) data.size(), (int) max_sz);

	if (res.second == false)
	{
		/* pop */

		if (res.first->second.next != data.end())
		{
			res.first->second.next->second.prev = res.first->second.prev;
		}
		else
		{
			return res.first->second.elem;
		}

		if (res.first->second.prev != data.end())
		{
			res.first->second.prev->second.next = res.first->second.next;
		}
		else
		{
			beg_it = res.first->second.next;
		}

		res.first->second.next = data.end();
		res.first->second.prev = end_it;

		/* push_back */

		end_it->second.next = res.first;
		end_it = res.first;

		return res.first->second.elem;
	}

	res.first->second.prev = end_it;

	if (end_it != data.end())
	{
		end_it->second.next = res.first;
	}

	end_it = res.first;

	if (beg_it == data.end())
	{
		beg_it = end_it;
	}
	else if (data.size() > size)
	{
		typename data_t::iterator beg_it_old = beg_it;
		beg_it = beg_it->second.next;

		if (beg_it != data.end())
		{
			beg_it->second.prev = data.end();
		}

		data.erase(beg_it_old);
	}

	return res.first->second.elem;
}

template <typename Key, typename Val>
void coda_cache<Key, Val>::dbg()
{
	for (typename data_t::iterator it = beg_it; it != data.end(); it = it->second.next)
	{
		std::cerr << it->first << std::endl;
	}
}
