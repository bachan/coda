#ifndef MYSQL_DATABASE_H
#define MYSQL_DATABASE_H


#include <stdint.h>
#include <string.h>

#include <string>
#include <map>
#include <vector>

#include "logevent.h"

namespace mysql {
	

class IItem {
public:
	IItem() {
	}
	virtual ~IItem() throw() {
	}
	
	virtual bool is_container() {
		return false;
	}
	
};


/* 
 * ========================================= CValue
 * ========================================================
 */	
class CValue : public IItem
{
public:
	enum EColumnType {
		MYSQL_TYPE_DECIMAL, 
		MYSQL_TYPE_TINY,
		MYSQL_TYPE_SHORT,  
		MYSQL_TYPE_LONG,
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
		MYSQL_TYPE_VARCHAR,
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
	
	static int calc_field_size(CValue::EColumnType ftype, uint8_t *pfield, uint32_t metadata);
	
public:
	CValue();
	CValue(const CValue& val);
	virtual ~CValue() throw();

	CValue& operator=(const CValue &val);
	bool operator==(const CValue &val) const;
	bool operator!=(const CValue &val) const;
	
	void reset(EColumnType type);
	bool is_valid() const;
	
public:
	EColumnType _type;
	int _position;
	
protected:
	size_t _size;
	const char *_storage;
	uint32_t _metadata;
	bool _is_null;
};


/* 
 * ========================================= CItem
 * ========================================================
 */	
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
	CContainer() {
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

	virtual IItem* watch(std::string name) = 0;

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



/* 
 * ========================================= CDatabase
 * ========================================================
 */	
class CDatabase : public CContainer
{
public:
	IItem* watch(std::string name);
};


/* 
 * ========================================= CTable
 * ========================================================
 */	
class CTable : public CContainer, public CTableMapLogEvent
{
public:
//	typedef std::map<std::string, CValue::EColumnType> TColumnsByName;
	typedef std::vector<CValue*> TRow;
public:
	CTable();
	CTable(CDatabase *db);
	virtual ~CTable() throw();
	
	CDatabase* db() const {
		return _db;
	}
	void db(CDatabase *db) {
		_db = db;
	}
	
	int change_values(CRowLogEvent &rlev);
	virtual IItem* watch(std::string name);
	virtual int tune(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt);
	virtual bool is_valid() const;
	
	int build_column(int position, const char *name);
	
	TRow& get_values() {
		return _values;
	}
	
	CValue& operator[](int idx);
	
	
protected:
	TItems _all_items;
	CDatabase *_db;
	TRow _values;
	bool _tuned;
	CValue _null_value;
};	
	



}

#endif // DATABASE_H
