template <typename _T, unsigned int Max>
io::ostream& operator << (io::ostream& o, const coda::log_array<_T, Max> &a)
{
	uint32_t header = 0x10600000 + Max;
	o << header;

	for (unsigned i = 0; i < Max; i++)
	{
		const _T &t = a.get_at(i); 
		o << t;
	}

	return o;
}

template <typename _T, unsigned int Max>
io::istream& operator >> (io::istream& i, coda::log_array<_T, Max> &a)
{
	uint32_t header;
	i >> header;

	if (header != 0x10600000 + Max)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "Error reading %s: coda::log_array checkcode incorrect.", i.name().c_str());
		throw std::logic_error(errbuf);
	}

	for (unsigned it = 0; it < Max; it++)
	{
		_T &t = a.get_at(it);
		i >> t;
	}

	return i;
}

template <typename _T, unsigned Max>
void save_tr1_array(io::ostream& o, const std::tr1::array<_T, Max> &a)
{
	uint32_t header = 0xD2100000 + Max;
	o << header;

	for (unsigned i = 0; i < Max; i++)
	{
		const _T &t = a[i];
		o << t;
	}
}

template <typename _T, unsigned Max>
void load_tr1_array(io::istream& i, std::tr1::array<_T, Max> &a)
{
	uint32_t header;
	i >> header;

	if (header != 0xD2100000 + Max)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "Error reading %s: std::tr1::array checkcode incorrect.", i.name().c_str());
		throw std::logic_error(errbuf);
	}

	for (unsigned it = 0; it < Max; it++)
	{
		_T &t = a[it];
		i >> t;
	}
}

template <typename _T>
io::ostream& operator << (io::ostream& o, const std::map<std::string, _T> &vm)
{
	uint32_t header = 0xF8AB1100;
	o.write(&header, sizeof(uint32_t));

	uint32_t sz = vm.size();
	o.write((void*)&sz, sizeof(uint32_t));

	typename std::map<std::string, _T>::const_iterator it;
	for (it = vm.begin(); it != vm.end(); it++)
	{
		uint8_t k_sz = it->first.size();
		o.write((void*)&k_sz, sizeof(uint8_t));
		o.write((void*)it->first.c_str(), k_sz);
		o << it->second;
	}

	return o;
}

template <typename _T>
io::ostream& operator << (io::ostream& o, const std::tr1::unordered_map<uint32_t, _T>& vm)
{
	uint32_t header = 0xF6BA1200;
	o.write(&header, sizeof(uint32_t));

	uint32_t sz = vm.size();
	o.write((void*)&sz, sizeof(uint32_t));

	typename std::tr1::unordered_map<uint32_t, _T>::const_iterator it;
	for (it = vm.begin(); it != vm.end(); it++)
	{
		o << it->first;
		o << it->second;
	}
	
	return o;
}

//google::sparse_hash_map

template <typename _T>
io::ostream& operator << (io::ostream& o, const google::sparse_hash_map<uint32_t, _T>& vm)
{
	uint32_t header = 0xF6BA1200;
	o.write(&header, sizeof(uint32_t));

	uint32_t sz = vm.size();
	o.write((void*)&sz, sizeof(uint32_t));

	typename google::sparse_hash_map<uint32_t, _T>::const_iterator it;
	for (it = vm.begin(); it != vm.end(); it++)
	{
		o << it->first;
		o << it->second;
	}
	
	return o;
}


template <typename _T>
io::ostream& operator << (io::ostream& o, const std::tr1::unordered_map<uint64_t, _T>& vm)
{
	uint32_t header = 0xF6BA6400;
	o.write(&header, sizeof(uint32_t));

	uint32_t sz = vm.size();
	o.write((void*)&sz, sizeof(uint32_t));

	typename std::tr1::unordered_map<uint64_t, _T>::const_iterator it;
	for (it = vm.begin(); it != vm.end(); it++)
	{
		o << it->first;
		o << it->second;
	}
	
	return o;
}

/////////////////////////////////////////
template <typename _T>
io::istream& operator >> (io::istream& i, std::map<std::string, _T> &vm)
{
	uint32_t header;
	i.read(&header, sizeof(uint32_t));
	if (header != 0xF8AB1100)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "Error reading %s: io::vptslist checkcode incorrect.", i.name().c_str());
		throw std::logic_error(errbuf);
	}

	uint32_t sz;
	i.read(&sz, sizeof(uint32_t));

	while (sz--)
	{
		uint8_t len;
		char buf[256];
		i.read(&len, sizeof(uint8_t));
		i.read(buf, len);
		buf[len] = 0;
		i >> vm[buf];
	}
	return i;
}

template <typename _T>
io::istream& operator >> (io::istream& i, std::tr1::unordered_map<uint32_t, _T> &vm)
{
	uint32_t header;
	i.read(&header, sizeof(uint32_t));
	if (header != 0xF6BA1200)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "Error reading %s: io::vptslist checkcode incorrect.", i.name().c_str());
		throw std::logic_error(errbuf);
	}

	uint32_t sz;
	i.read(&sz, sizeof(uint32_t));

	while (sz--)
	{
		uint32_t key;

		i >> key;
		_T &val = vm[key];
		i >> val;
	}

	return i;
}

template <typename _T>
io::istream& operator >> (io::istream& i, google::sparse_hash_map<uint32_t, _T> &vm)
{
	uint32_t header;
	i.read(&header, sizeof(uint32_t));
	if (header != 0xF6BA1200)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "Error reading %s: io::vptslist checkcode incorrect.", i.name().c_str());
		throw std::logic_error(errbuf);
	}

	uint32_t sz;
	i.read(&sz, sizeof(uint32_t));

	while (sz--)
	{
		uint32_t key;

		i >> key;
		_T &val = vm[key];
		i >> val;
	}

	return i;
}

template <typename _T>
io::istream& operator >> (io::istream& i, std::tr1::unordered_map<uint64_t, _T> &vm)
{
	uint32_t header;
	i.read(&header, sizeof(uint32_t));
	if (header != 0xF6BA6400)
	{
		char errbuf[1024];
		snprintf(errbuf, 1024, "Error reading %s: io::vptslist checkcode incorrect.", i.name().c_str());
		throw std::logic_error(errbuf);
	}

	uint32_t sz;
	i.read(&sz, sizeof(uint32_t));

	while (sz--)
	{
		uint64_t key;

		i >> key;
		_T &val = vm[key];
		i >> val;
	}

	return i;
}

