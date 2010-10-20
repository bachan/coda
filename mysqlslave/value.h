#ifndef MYSQL_VALUE_H
#define MYSQL_VALUE_H

#include <stdint.h>
#include <stdio.h>
#include <strings.h>
#include <string>
#include <map>
#include <stdexcept>

#include "logevent.h"

namespace mysql 
{

class IItem 
{
public:
	IItem() 
		: _id(0)
	{
	}
	IItem(uint64_t id)
		: _id(id)
	{
	}
	virtual ~IItem() throw() {
	}
	
	uint64_t id() const {
		return _id;
	}
	void id(uint64_t id) {
		_id = id;
	}
protected:
	uint64_t _id;
};
	

class CContainer : public IItem
{
protected:
	class _items_nocase_comparer {
		public: bool operator() (const std::string &s1, const std::string &s2) const {
			return strcasecmp(s1.c_str(), s2.c_str()) > 0;	
		}
	};
	typedef std::map<std::string, IItem*, CContainer::_items_nocase_comparer> TItems;
public:
	CContainer() 
	{
	}
	CContainer(uint64_t id)
		: IItem(id)
	{
	}
	
	virtual ~CContainer() throw() {
		try 
		{
			for( TItems::iterator it = _watched_items.begin(); it != _watched_items.end(); ++it )
				if( it->second )
				{
					delete it->second;
					it->second = NULL;
				}
		}
		catch(...)
		{
			;
		}
	}

	virtual IItem* watch(std::string name, uint64_t id = 0) {
		IItem *item = find(&name);
		if( !item )
		{
			item = new IItem(id);
			_watched_items[name] = item;
		}
		return item;
	}

	IItem* find(std::string name) {
		return this->find(&name);
	}

	IItem* find(std::string *name) {
		IItem *rc;
		TItems::iterator it = _watched_items.find(*name);
		rc = it != _watched_items.end() ? it->second : NULL;
		return rc;
	}
	
protected:
	TItems _watched_items;
};



class CValue : public IItem
{
public:
	enum EColumnType {
		MYSQL_TYPE_DECIMAL, 
		MYSQL_TYPE_TINY,
		MYSQL_TYPE_SHORT,  
		MYSQL_TYPE_LONG = 3,
		MYSQL_TYPE_FLOAT,  
		MYSQL_TYPE_DOUBLE,
		MYSQL_TYPE_NULL,
		MYSQL_TYPE_TIMESTAMP,
		MYSQL_TYPE_LONGLONG,
		MYSQL_TYPE_INT24,
		MYSQL_TYPE_DATE,
		MYSQL_TYPE_TIME,
		MYSQL_TYPE_DATETIME, 
		MYSQL_TYPE_YEAR,
		MYSQL_TYPE_NEWDATE,
		MYSQL_TYPE_VARCHAR = 15,
		MYSQL_TYPE_BIT,
		MYSQL_TYPE_NEWDECIMAL=246,
		MYSQL_TYPE_ENUM=247,
		MYSQL_TYPE_SET=248,
		MYSQL_TYPE_TINY_BLOB=249,
		MYSQL_TYPE_MEDIUM_BLOB=250,
		MYSQL_TYPE_LONG_BLOB=251,
		MYSQL_TYPE_BLOB=252,
		MYSQL_TYPE_VAR_STRING=253,
		MYSQL_TYPE_STRING=254,
		MYSQL_TYPE_GEOMETRY=255
	};
	static int calc_metadata_size(CValue::EColumnType ftype);
	static int calc_field_size(CValue::EColumnType ftype, const uint8_t *pfield, uint32_t metadata);

public:
	CValue();
	CValue(const CValue& val);
	virtual ~CValue() throw();

	CValue& operator=(const CValue &val);
	bool operator==(const CValue &val) const;
	bool operator!=(const CValue &val) const;
	
	
	
	int64_t as_int() const;
	uint64_t as_uint() const;
	time_t as_time() const;
	const char* const as_c_str(size_t *length) const;
	std::string as_std_str() const;
	
	
	void reset();
	int tune(CValue::EColumnType ftype, const uint8_t *pfield, uint32_t metadata, size_t length);
	bool is_valid() const;
	inline bool is_null() const { 
		return _is_null; 
	}
	
public:
	EColumnType _type;
protected:
	const uint8_t *_storage;
	size_t _size;
	uint32_t _metadata;
	bool _is_null;
};

}
#endif

