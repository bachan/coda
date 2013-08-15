#include <sstream> /* this is needed for dbg() output */

template <typename Key, typename Val>
const Val *coda_cache<Key, Val>::get(const Key &key) const
{
	typename data_t::const_iterator it = data.find(key);

	if (it != data.end())
	{
		if (time_max && it->second.time + time_max < (size_t) time(NULL))
		{
			return NULL;
		}
		else
		{
			return &it->second.elem;
		}
	}

	return NULL;
}

template <typename Key, typename Val>
void coda_cache<Key, Val>::get_copy(const Key &key, Val &val)
{
	typename data_t::iterator it = data.find(key);

	if (it != data.end())
	{
		if (time_max && it->second.time + time_max < (size_t) time(NULL))
		{
			erase(it);
		}
		else
		{
			val = it->second.elem;
		}
	}
}

template <typename Key, typename Val>
void coda_cache<Key, Val>::set(const Key &key, const Val &val, bool do_update_time)
{
	std::pair<typename data_t::iterator, bool> res;

	res = data.insert(typename data_t::value_type(key, elem_t(data.end(), data.end())));

	if (res.second == false)
	{
		/* pop */

		if (res.first->second.next != data.end())
		{
			res.first->second.next->second.prev = res.first->second.prev;
		}
		else
		{
			size_cur -= res.first->second.size;
			res.first->second.elem = val;
			res.first->second.size = estimate_capacity(val);
			size_cur += res.first->second.size;

			if (do_update_time)
			{
				res.first->second.time = time(NULL);
			}

			return;
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

		size_cur -= res.first->second.size;
		res.first->second.elem = val;
		res.first->second.size = estimate_capacity(val);
		size_cur += res.first->second.size;

		if (do_update_time)
		{
			res.first->second.time = time(NULL);
		}

		return;
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

	res.first->second.elem = val;
	res.first->second.size = estimate_capacity(val);
	size_cur += res.first->second.size;

	drop_last_unused();
}

template <typename Key, typename Val>
bool coda_cache<Key, Val>::erase(typename data_t::iterator it)
{
	if (it == data.end())
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
bool coda_cache<Key, Val>::erase(const Key &key)
{
	return erase(data.find(key));
}

template <typename Key, typename Val>
void coda_cache<Key, Val>::drop_last_unused()
{
	if (beg_it == data.end()) return;

	while (size_cur > size_max)
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
void coda_cache<Key, Val>::dbg(std::string &res, int dbg) const
{
	size_t time_cur = time(NULL);

	coda_strappend(res, "nelt_cur=%"PRIuMAX"\n", (uintmax_t) data.size());
	coda_strappend(res, "size_cur=%"PRIuMAX"\n", (uintmax_t) size_cur);
	coda_strappend(res, "size_max=%"PRIuMAX"\n", (uintmax_t) size_max);
	coda_strappend(res, "time_max=%"PRIuMAX"\n", (uintmax_t) (time_cur - time_max));
	coda_strappend(res, "\n");

	if (!dbg) return;

	std::stringstream os;

	for (typename data_t::const_iterator it = beg_it; it != data.end(); it = it->second.next)
	{
		os
			<< it->first << '\t'
			<< it->second.size << '\t'
			<< &*(it->second.prev) << '-'
			<< &*(it->second.next) << '\t'
			<< it->second.time << '\n'
		;
	}

	res.append(os.str());
}

