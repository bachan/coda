#include "cache.hpp"

template <typename Key, typename Val>
Val& coda_cache_memlim<Key, Val>::acquire(const Key &key)
{
	std::pair<typename data_t::iterator, bool> res;

	res = data.insert(typename data_t::value_type(key, elem_t(data.end(), data.end())));

	/* clear outdated element */
	size_t time_cur = time(NULL);
	if (res.first->second.time + time_max < time_cur &&
		res.first->second.link == 0)
	{
		res.first->second.elem = Val();
		res.first->second.time = time_cur;
	}

	res.first->second.link++;

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

	return res.first->second.elem;
}

template <typename Key, typename Val>
void coda_cache_memlim<Key, Val>::release(const Key &key)
{
	typename data_t::iterator it = data.find(key);

	if (it != data.end() && it->second.link > 0)
	{
		it->second.link--;

		size_cur -= it->second.size;
		it->second.size = it->second.elem.size();
		size_cur += it->second.size;
	}

	drop_last_unused();
}

template <typename Key, typename Val>
bool coda_cache_memlim<Key, Val>::drop(const Key &key)
{
	typename data_t::iterator it = data.find(key);

	if (it == data.end() || it->second.link > 0)
	{
		return false;
	}

	if (it->second.next != data.end())
	{
		it->second.next->second.prev = it->second.prev;
	}
	else
	{
		end_it = it->second.prev;
	}

	if (it->second.prev != data.end())
	{
		it->second.prev->second.next = it->second.next;
	}
	else
	{
		beg_it = it->second.next;
	}

	size_cur -= it->second.size;
	data.erase(it);

	return true;
}

template <typename Key, typename Val>
void coda_cache_memlim<Key, Val>::drop_last_unused()
{
	if (beg_it == data.end()) return;

	while (size_cur > size_max && 0 == beg_it->second.link)
	{
		typename data_t::iterator beg_it_old = beg_it;
		beg_it = beg_it->second.next;

		if (beg_it != data.end())
		{
			beg_it->second.prev = data.end();
		}
		else
		{
			end_it = beg_it;
		}

		size_cur -= beg_it_old->second.size;
		data.erase(beg_it_old);
	}
}

template <typename Key, typename Val>
void coda_cache_memlim<Key, Val>::dbg(std::string &res, int dbg)
{
	size_t time_cur = time(NULL);

	coda_strappend(res, "nelt_cur=%"PRIuMAX"\n", (uintmax_t) data.size());
	coda_strappend(res, "size_cur=%"PRIuMAX"\n", (uintmax_t) size_cur);
	coda_strappend(res, "size_max=%"PRIuMAX"\n", (uintmax_t) size_max);
	coda_strappend(res, "time_max=%"PRIuMAX"\n", (uintmax_t) (time_cur - time_max));
	coda_strappend(res, "\n");

	if (!dbg) return;

	for (typename data_t::iterator it = beg_it; it != data.end(); it = it->second.next)
	{
		coda_strappend(res, "%s,%"PRIuMAX",%p-%p,%"PRIuMAX",%"PRIuMAX"\n",
			it->first.c_str(),
			(uintmax_t) it->second.size,
			&*(it->second.prev),
			&*(it->second.next),
			(uintmax_t) it->second.link,
			(uintmax_t) it->second.time
		);
	}
}

