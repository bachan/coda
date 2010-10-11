#ifndef MYSQL_DATABASE_H
#define MYSQL_DATABASE_H


#include <stdint.h>
#include <string>
#include <map>

namespace mysql {
	
class CDatabase;

	
class CValue 
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

protected:
	EColumnType _type;
	size_t _size;
	const char *_storage;
	uint32_t _metadata;
	bool _is_null;
	
};


class CTable 
{
public:
	typedef std::map<std::string, CValue::EColumnType> TColumnsByName;
public:
	CTable();
	CTable(CDatabase *db, std::string &name);
	virtual ~CTable() throw();
	
	uint64_t id() const {
		return _id;
	}
	void id(uint64_t id) {
		_id = id;
	}
	
	const std::string& name() const {
		return _name;
	}
	void id(std::string &name) {
		_name = name;
	}

	CDatabase* db() const {
		return _db;
	}
	void db(CDatabase *db) {
		_db = db;
	}
	

protected:
	CDatabase* _db;
	std::string _name;
	uint64_t _id;
};	
	

class CDatabase
{
public:
	typedef std::map<std::string, CTable*> TTablesByName;
	typedef std::map<uint64_t, CTable*> TTablesByID;
public:
    CDatabase();
    virtual ~CDatabase() throw();
	
	const std::string& name() const {
		return _name;
	}
	void name(std::string &name) {
		_name = name;
	}
	void id(std::string &name) {
		_name = name;
	}
	
	CTable* monitor_table(std::string table_name);
protected:
	std::string _name;
	TTablesByName _tbl_by_name;
	TTablesByID _tbl_by_id;
};


}

#endif // DATABASE_H
