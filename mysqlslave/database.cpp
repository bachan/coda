
#include "database.h"

namespace mysql {

/* 
 * ========================================= CTable
 * ========================================================
 */	
	
CTable::CTable()
	: _db(NULL)
	, _tuned(false)
{
}

CTable::CTable(CDatabase *db) 
	: _db(db)
	, _tuned(false)
{
}

CTable::~CTable() throw()
{
}


int CTable::tune(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt)
{
	if( _tuned )
		return 0;

	int rc = CTableMapLogEvent::tune(data, size, fmt);
	if( rc == 0 && CTableMapLogEvent::is_valid() )
	{
		_row.indexer(this);
		_row.resize(_column_count);
		uint8_t *type = _metadata;
		for( int i=0; i<_column_count; ++i )
		{
			_row[i].id(i);
			_row[i]._type = (CValue::EColumnType)*type++;
		}
		_tuned = true;
	}
	
	return rc;
}

int CTable::update(CRowLogEvent &rlev)
{
	const uint8_t *pfields;
	size_t len;
	uint64_t nullfields_mask;
	
	if( !rlev.is_valid() || _table_id != rlev._table_id)
		return -1;
	
	pfields = rlev.rows_data();
	len = rlev.rows_len();
	
	_new_rows.clear();
	_rows.clear();
	while( len > 0 )
	{
		nullfields_mask = ~rlev.build_column_mask(&pfields, &len, rlev.get_used_columns_1bit_count());
		update_row(_row, &pfields, &len, 
				   rlev._ncolumns, rlev.get_used_columns_mask(), nullfields_mask);
		_rows.push_back(_row);
		if( rlev.get_type_code() == UPDATE_ROWS_EVENT )
		{
			nullfields_mask = ~rlev.build_column_mask(&pfields, &len, rlev.get_used_columns_afterimage_1bit_count());
			update_row(_row, &pfields, &len, 
					   rlev._ncolumns, rlev.get_used_columns_afterimage_mask(), nullfields_mask);
			_new_rows.push_back(_row);
		}
	}
		
	return len == 0 ? 0 : -1;
}

int CTable::update_row(CRow &row, const uint8_t **pdata, size_t *len, 
				uint64_t ncolumns, uint64_t usedcolumns_mask, uint64_t nullfields_mask)
{
	CValue::EColumnType type;
	uint32_t metadata;
	uint32_t length;
	uint8_t *pmetadata;
	uint64_t bit;
	
	bit = 0x01;
	pmetadata = _metadata;
	for(uint64_t i = 0; i<ncolumns && *len > 0; ++i)
	{
		type = (CValue::EColumnType)*(_column_types+i);
		switch( CValue::calc_metadata_size(type) )
		{
			case 0:
			{
				metadata = 0;
				break;
			}
			case 1:
			{
				metadata = *pmetadata;
				pmetadata++;
				break;
			}
			case 2:
			{
				metadata = *(uint16_t*)pmetadata;
				pmetadata += 2;
				break;
			}
			default:
				// хз, ненадежно, конечно, но метаданные по протоколу типа не должны быть длиннее 2 байт
				metadata = 0; 
		}
		
		row[i].reset();
		if( usedcolumns_mask & nullfields_mask & bit)
		{
			length = CValue::calc_field_size(type, *pdata, metadata);
			row[i].tune(type, *pdata, metadata, length);
			(*pdata) += length;
			*len -= length;
		}
			
		bit << 1;
	}
	
	return *len >= 0 ? 0 : -1;
}

/* 
 * ========================================= CDatabase
 * ========================================================
 */	

IItem* CDatabase::watch(std::string name, uint64_t id)
{
	IItem *table = find(&name);
	if( !table )
	{
		table = new CTable(this);
		_watched_items[name] = table;
	}
	return table;//tbl;
}



}