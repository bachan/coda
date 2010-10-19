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
	
	void reset(EColumnType type);
	bool is_valid() const;
	
	bool updated() const {
		return _is_updated;
	}
	void updated(bool upd)  {
		_is_updated = upd;
	}
	bool zero() const {
		return _is_null;
	}
	void zero(bool z) {
		_is_null = z;
	}
	
	
	uint64_t as_int64() const 
	{
		return *((uint32_t*)_storage);
	}
	int tune(CValue::EColumnType ftype, const uint8_t *pfield, uint32_t metadata, size_t length)
	{
		_type = ftype;
		_storage = pfield;
		_metadata = metadata;
		_size = length;
		printf("field type: %d, length: %d, metadata: %d, uint32_t cast: %d\n", 
			   (int)_type, (int)length, (int)metadata, (int)*(uint32_t*)pfield);
		return 0;
	}
	
public:
	EColumnType _type;
	int _position;
	
protected:
	size_t _size;
	const uint8_t *_storage;
	uint32_t _metadata;
	bool _is_null;
	bool _is_updated;
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
	typedef std::vector<TRow> TRows;
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
	
	TRows& get_new_rows() {
		return _new_rows;
	}
	TRows& get_old_rows() {
		return _old_rows;
	}
	
	
	CValue& operator[](int idx);
protected:
	int update_row(TRow &row, const uint8_t **pdata, size_t *len, 
				   uint64_t ncolumns, uint64_t usedcolumns_mask, uint64_t nullfields_mask);
	
protected:
	TItems _all_items;
	CDatabase *_db;
	TRow _values;
	TRow _new_values;
	TRows _new_rows;
	TRows _old_rows;
	bool _tuned;
	CValue _null_value;
};	
	



}

#endif // DATABASE_H
