
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
	: _size(0)
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
	

/* 
 * ========================================= CTable
 * ========================================================
 */	
	
CTable::CTable()
	: _columns(_items)
	, _db(NULL)
	, _id(0)
{
}

CTable::CTable(CDatabase *db, std::string &name) 
	: CItem(name) 
	, _columns(_items)
	,_db(db)
	, _id(0)
{
}

CTable::~CTable() throw()
{
}

CItem* CTable::watch(std::string name)
{
//	std::pair<TItems::iterator, int> rc = 
	_columns.insert(std::pair<std::string, CItem*>(name, NULL));
	return NULL;
}



/* 
 * ========================================= CDatabase
 * ========================================================
 */	

CItem* CDatabase::watch(std::string name)
{
	CItem *table = find(name);
	if( !table )
	{
		table = new CTable(this, name);
		_tables[name] = table;
	}
	return table;//tbl;
}



}