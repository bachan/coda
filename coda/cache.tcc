template <typename Key, typename Val>
void coda_cache<Key, Val>::get_copy(const Key &key, Val &val)
{
	typename data_t::iterator it = data.find(key);

	if (it != data.end())
	{
		size_t time_cur = time(NULL);

		if (it->second.time + time_max < time_cur)
		{
			drop(it);
		}
		else
		{
			val = it->second.elem;
		}
	}
}

template <typename Key, typename Val>
void coda_cache<Key, Val>::set(const Key &key, const Val &val)
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
			// return res.first->second.elem;

			size_cur -= res.first->second.size;
			res.first->second.elem = val;
			res.first->second.size = val.size();
			size_cur += res.first->second.size;

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

		// return res.first->second.elem;

		size_cur -= res.first->second.size;
		res.first->second.elem = val;
		res.first->second.size = val.size();
		size_cur += res.first->second.size;

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

	// return res.first->second.elem;

	res.first->second.elem = val;
	res.first->second.size = val.size();
	size_cur += res.first->second.size;

	// ...

	drop_last_unused();
}

template <typename Key, typename Val>
const Val *coda_cache<Key, Val>::get(const Key &key) const
{
	typename data_t::const_iterator it = data.find(key);
	return it != data.end() ? &it->second.elem : NULL;
}

template <typename Key, typename Val>
void coda_cache<Key, Val>::update_time(const Key &key)
{
	typename data_t::iterator it = data.find(key);
	if (it != data.end()) it->second.time = ::time(NULL);
}

template <typename Key, typename Val>
bool coda_cache<Key, Val>::drop(typename data_t::iterator it)
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
bool coda_cache<Key, Val>::drop(const Key &key)
{
	return drop(data.find(key));
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

	for (typename data_t::const_iterator it = beg_it; it != data.end(); it = it->second.next)
	{
		coda_strappend(res, "%s\t%"PRIuMAX"\t%p-%p\t%"PRIuMAX"\n",
			it->first.c_str(),
			(uintmax_t) it->second.size,
			&*(it->second.prev),
			&*(it->second.next),
			(uintmax_t) it->second.time
		);
	}
}

