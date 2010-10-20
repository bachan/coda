#include "value.h"

namespace mysql
{
/* 
 * ========================================= CValue
 * ========================================================
 */	
int CValue::calc_metadata_size(CValue::EColumnType ftype)
{
	int rc;
	switch( ftype ) 
	{
	case MYSQL_TYPE_FLOAT:
	case MYSQL_TYPE_DOUBLE:
	case MYSQL_TYPE_BLOB:
	case MYSQL_TYPE_GEOMETRY:
		rc = 1;
		break;
	case MYSQL_TYPE_VARCHAR:
	case MYSQL_TYPE_BIT:
	case MYSQL_TYPE_NEWDECIMAL:
	case MYSQL_TYPE_VAR_STRING:
	case MYSQL_TYPE_STRING:
		rc = 2;
		break;
	default:
		rc = 0;
	}
	return rc;
}
	
int CValue::calc_field_size(CValue::EColumnType ftype, const uint8_t *pfield, uint32_t metadata)
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
	, _storage(NULL)
	, _size(0)
	, _metadata(0)
	, _is_null(true)
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
	this->_type = val._type;
	this->_storage = val._storage;
	this->_size = val._size;
	this->_metadata = val._metadata;
	this->_is_null = val._is_null;
	return *this;
}

bool CValue::operator==(const CValue &val) const
{
	return 
	(_type == val._type) &&
	(_storage == val._storage) &&
	(_size == val._size) &&
	(_metadata == val._metadata) &&
	(_is_null == val._is_null)
	;
			
}

bool CValue::operator!=(const CValue &val) const
{
	return !operator==(val);
}
	
void CValue::reset() 
{
	_size = 0;
	_storage = NULL;
	_metadata = 0;
	_is_null = true;
}

bool CValue::is_valid() const 
{
	return _size && _storage;
}

int64_t CValue::as_int() const
{
	if( _is_null || !is_valid() )
		return 0;

	int64_t rc = 0;
	switch( _size )
	{
	case 1:
	{
		rc = (int64_t)*_storage;
		break;
	}
	case 2:
	{
		rc = (int64_t)sint2korr(_storage);
		break;
	}
	case 3:
	{
		rc = (int64_t)sint3korr(_storage);
		break;
	}
	case 4:
	{
		rc = (int64_t)sint4korr(_storage);
		break;
	}
	case 8:
	{
		rc = (int64_t)sint8korr(_storage);
	}
	}
	return rc;
}
uint64_t CValue::as_uint() const 
{
	return (uint64_t)as_int();
}


time_t CValue::as_time() const
{
	if( _is_null )
		return 0;
	
	
	return 0;
}


const char* const CValue::as_c_str(size_t *length) const
{
	if( _is_null || _type != MYSQL_TYPE_VARCHAR )
	{
		*length = 0;
		return NULL;
	}
	const uint8_t *pstr = _storage;
	*length = net_field_length(const_cast<uint8_t**>(&pstr));
	return (const char*)pstr;
}

std::string CValue::as_std_str() const
{
	size_t len;
	const char *cstr = as_c_str(&len);
	if( cstr && len > 0 )
		return std::string().assign(cstr, len);
	
	return std::string("");
}

int CValue::tune(CValue::EColumnType ftype, const uint8_t *pfield, uint32_t metadata, size_t length)
{
	_type = ftype;
	_storage = pfield;
	_size = length;
	_metadata = metadata;
	_is_null = false;
/*	printf("field type: %d, length: %d, metadata: %d, uint32_t cast: %d\n", 
			(int)_type, (int)length, (int)metadata, (int)*(uint32_t*)pfield);*/
	return 0;
}






}