
#include "database.h"

namespace mysql {

/* 
 * ========================================= CValue
 * ========================================================
 */	
	
int CValue::calc_field_size(CValue::EColumnType ftype, uint8_t *pfield, uint32_t metadata)
{
	uint32_t length;

	switch (ftype) {
	case MYSQL_TYPE_VAR_STRING:
	/* This type is hijacked for result set types. */
		length= metadata;
		break;
	case MYSQL_TYPE_NEWDECIMAL:
	//length= my_decimal_get_binary_size(metadata_ptr[col] >> 8,
	//                                   metadata_ptr[col] & 0xff);
		length= 0;
		break;
	case MYSQL_TYPE_DECIMAL:
	case MYSQL_TYPE_FLOAT:
	case MYSQL_TYPE_DOUBLE:
		length= metadata;
		break;
	/*
	The cases for SET and ENUM are include for completeness, however
	both are mapped to type MYSQL_TYPE_STRING and their real types
	are encoded in the field metadata.
	*/
	case MYSQL_TYPE_SET:
	case MYSQL_TYPE_ENUM:
	case MYSQL_TYPE_STRING:
	{
		unsigned char type= metadata >> 8U;
		if ((type == MYSQL_TYPE_SET) || (type == MYSQL_TYPE_ENUM))
			length= metadata & 0x00ff;
		else
		{
			/*
			We are reading the actual size from the master_data record
			because this field has the actual lengh stored in the first
			byte.
			*/
			length = (unsigned int) *pfield+1;
		}
		break;
	}
	case MYSQL_TYPE_YEAR:
	case MYSQL_TYPE_TINY:
		length = 1;
		break;
	case MYSQL_TYPE_SHORT:
		length = 2;
		break;
	case MYSQL_TYPE_INT24:
		length = 3;
		break;
	case MYSQL_TYPE_LONG:
		length = 4;
		break;
	//  case MYSQL_TYPE_LONGLONG:
	//    length= 8;
	//    break;
	case MYSQL_TYPE_NULL:
		length = 0;
		break;
	case MYSQL_TYPE_NEWDATE:
	case MYSQL_TYPE_DATE:
	case MYSQL_TYPE_TIME:
		length = 3;
		break;
	case MYSQL_TYPE_TIMESTAMP:
		length = 4;
		break;
	case MYSQL_TYPE_DATETIME:
		length = 8;
		break;
	case MYSQL_TYPE_BIT:
	{
		/*
		Decode the size of the bit field from the master.
		from_len is the length in bytes from the master
		from_bit_len is the number of extra bits stored in the master record
		If from_bit_len is not 0, add 1 to the length to account for accurate
		number of bytes needed.
		*/
		uint32_t from_len = (metadata >> 8U) & 0x00ff;
		uint32_t from_bit_len = metadata & 0x00ff;
		length = from_len + ((from_bit_len > 0) ? 1 : 0);
		break;
	}
	case MYSQL_TYPE_VARCHAR:
	{
		length = metadata > 255 ? 2 : 1;
		length += length == 1 ? (uint32_t)*pfield : *((uint16_t *)pfield);
		break;
	}
	case MYSQL_TYPE_TINY_BLOB:
	case MYSQL_TYPE_MEDIUM_BLOB:
	case MYSQL_TYPE_LONG_BLOB:
	case MYSQL_TYPE_BLOB:
	case MYSQL_TYPE_GEOMETRY:
	{
		switch (metadata)
		{
			case 1:
				length = 1 + (uint32_t) pfield[0];
				break;
			case 2:
				length = 2 + (uint32_t) (*(uint16_t*)(pfield) & 0xFFFF);
				break;
			case 3:
				length = 3 + (uint32_t) (long) (*((uint32_t *) (pfield)) & 0xFFFFFF);
				break;
			case 4:
				length = 4 + (uint32_t) (long) *((uint32_t *) (pfield));
				break;
			default:
				length= 0;
				break;
		}
		break;
	}
	default:
		length= ~(uint32_t) 0;
	}
	
	return length;
}
	
	
CValue::CValue()
	: _type(MYSQL_TYPE_NULL)
	, _position(0)
	, _size(0)
	, _storage(NULL)
	, _metadata(0)
	, _is_null(false)
{
}

CValue::CValue(const CValue& val)
{
	*this = val;
}

CValue::~CValue() throw()
{
}

CValue& CValue::operator=(const CValue &val)
{
	this->_size = val._size;
	this->_storage = val._storage;
	this->_type = val._type;
	this->_metadata = val._metadata;
	this->_is_null = val._is_null;
	return *this;
}

bool CValue::operator==(const CValue &val) const
{
	return (_size == val._size) &&
			(_storage == val._storage) &&
			(_type == val._type) &&
			(_metadata == val._metadata);
}

bool CValue::operator!=(const CValue &val) const
{
	return !operator==(val);
}
	
void CValue::reset(EColumnType type) 
{
	_type = type;
	_size = 0;
	_storage = NULL;
	_metadata = 0;
	_is_null = false;
}

bool CValue::is_valid() const 
{
	return _size && _storage;
}

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
	// free at inherited destructor
	TItems::iterator ita;
	for(TItems::iterator ita = _all_items.begin(); ita != _all_items.end(); ++ita)
		_watched_items.insert(std::pair<std::string, IItem*>(ita->first, ita->second));
}

IItem* CTable::watch(std::string name)
{
	IItem *value = find(&name);
	if( !value )
	{
		value = new CValue();
		_watched_items[name] = value;
	}
	return value;
}

int CTable::tune(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt)
{
	if( _tuned )
		return 0;

	_values.clear();
	int rc = CTableMapLogEvent::tune(data, size, fmt);
	if( rc || !_column_count || !_metadata )
	{
		uint8_t *type = _metadata;
		try
		{
			for( int i=0; i<_column_count; ++i )
				_values.at(i)->_type = (CValue::EColumnType)*type++;
			_tuned = true;
		}
		catch( ... )
		{
			rc = -1;
		}
			
	}
	else
		_tuned = false;
	
	return rc;
}

int CTable::change_values(CRowLogEvent &rlev)
{
	if( _table_id != rlev._table_id )
		return -1;
	
	// _all_items;
	
		
	
	
	return 0;
}

CValue& CTable::operator[](int idx)
{
	try
	{
		return *_values.at(idx);
	}
	catch(...)
	{
		;
	}
	return _null_value;
}

bool CTable::is_valid() const
{
	return CTableMapLogEvent::is_valid() && _column_count >= _watched_items.size();
}

int CTable::build_column(int position, const char *name)
{
	if( !name )
		return -1;
	
	CValue *value = static_cast<CValue*>(find(name));
	if( !value )
		value = new CValue();
	value->_position = position;
	_all_items.insert(std::pair<std::string, IItem*>(name, value));
	
	if( _values.size() < (position+1) )
		_values.resize(position+1);

	_values[position] = value;
}

/* 
 * ========================================= CDatabase
 * ========================================================
 */	

IItem* CDatabase::watch(std::string name)
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