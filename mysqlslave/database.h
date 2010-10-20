#ifndef MYSQL_DATABASE_H
#define MYSQL_DATABASE_H


#include <stdint.h>
#include <string.h>

#include <string>
#include <vector>
#include <list>

#include "logevent.h"
#include "value.h"

namespace mysql {


class CRow : public std::vector<CValue>
{
typedef std::vector<CValue> inherited;
public:
    CRow() : inherited(), _indexer(NULL) 
	{
	}
	CRow(CContainer *indexer) : inherited() , _indexer(indexer)
	{
	}

	const CValue& operator[](int idx) const { 
		return inherited::operator[](idx);
	}
	CValue& operator[](int idx) {
		return inherited::operator[](idx);
	}
	CValue& operator[](std::string name) {
		if( _indexer )
		{
			IItem *item = _indexer->find(name);
			if( item ) {
				try {
					return inherited::at(item->id());
				}
				catch( ... ) {
					;
				}
			}
		}
		return _null_value;
	}
	const CValue& operator[](std::string name) const {
		if( _indexer )
		{
			IItem *item = _indexer->find(name);
			if( item ) {
				try {
					return inherited::at(item->id());
				}
				catch( ... ) {
					;
				}
			}
		}
		return _null_value;
	}
	
	void indexer(CContainer *indexer) {
		_indexer = indexer;
	}
	CContainer* indexer() {
		return _indexer;
	}
	
protected:
	CContainer *_indexer;
	CValue _null_value;
};


/* 
 * ========================================= CTable
 * ========================================================
 */	
class CDatabase;
class CTable : public CContainer, public CTableMapLogEvent
{
public:
	typedef std::list<CRow> TRows;
public:
	CTable();
	CTable(CDatabase *db);
	virtual ~CTable() throw();
	
	virtual int tune(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt);
	virtual bool is_valid() const {
		return _tuned;
	}
	
	CDatabase* db() const {
		return _db;
	}
	void db(CDatabase *db) {
		_db = db;
	}
	const TRows& get_rows() const {
		return _rows;
	}
	const TRows& get_new_rows() const {
		return _new_rows;
	}
	
	int update(CRowLogEvent &rlev);
protected:
	int update_row(CRow &row, const uint8_t **pdata, size_t *len, 
				   uint64_t ncolumns, uint64_t usedcolumns_mask, uint64_t nullfields_mask);
	
protected:
	CDatabase *_db;
	CRow _row;
	TRows _rows;
	TRows _new_rows;
	bool _tuned;
};	

class CDatabase : public CContainer
{
public:
	IItem* watch(std::string name, uint64_t id);
};


}

#endif // DATABASE_H
