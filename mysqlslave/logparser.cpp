/*
 * logparser.cpp
 *
 *  Created on: 28.09.2010
 *      Author: azrahel
 */

#include "logparser.h"

namespace mysql {

CLogParser::CLogParser()
	: _databases(_watched_items)
	, _slave_id(0)
	, _port(0)
	, _fmt(0)
	, _binlog_pos(0)
	, _binlog_flags(0)
	, _err(0)
	, _dispatch(1)
{
	mysql_init(&_mysql);
}

CLogParser::~CLogParser() throw()
{
	disconnect();
}

int CLogParser::connect()
{
	disconnect();
	if( !mysql_init(&_mysql) )
	{
		on_error("mysql_init failed", 0);
		return -1;
	}

	if( !mysql_real_connect(&_mysql, _host.c_str(), _user.c_str(), _passwd.c_str(), 0, _port, 0, 0) )
	{
		on_error("mysql_real_connect() failed", &_mysql);
		return -1;
	}
	
	if( _fmt ) delete _fmt;
	_fmt = get_binlog_format();
	
	return _fmt ? 0 : 1;
}

int CLogParser::reconnect()
{
	int rc = connect();
	if ( rc == 0 )
	{
		rc = request_binlog_dump();
		if ( rc == 0 ) on_reconnect(&_mysql);
	}
	return rc;
}

void CLogParser::disconnect()
{
	mysql_close(&_mysql);
}


void CLogParser::set_connection_params(const char *host, uint32_t slave_id, const char *user, const char *passwd, int port)
{
	_host	= host;
	_user	= user;
	_passwd	= passwd;
	_port	= port;
	_slave_id = slave_id;
}

void CLogParser::set_binlog_position(const char *fname, uint32_t pos, uint32_t srv_id, uint16_t flags)
{
	_binlog_name = fname;
	_binlog_pos	= pos;
	_binlog_flags = flags;
}


CFormatDescriptionLogEvent* CLogParser::get_binlog_format()
{
	MYSQL_RES* res = 0;
	MYSQL_ROW row;
	const char* version;
	CFormatDescriptionLogEvent* fmt = 0;


	if( mysql_query(&_mysql, "SELECT VERSION()") || 
		!( res = mysql_store_result(&_mysql) ) ||
		!( row = mysql_fetch_row(res) ) || 
		!( version = row[0] )
	)
	{
		on_error("could not find server version", &_mysql);
		goto err;
	}

	if( *version != '5' )
	{
		on_error("invalid server version", 0);
		goto err;
	}

	 fmt = new CFormatDescriptionLogEvent(4);
	 if( !fmt || !fmt->is_valid() )
		 on_error("could not create CFormatDescriptionLogEvent", 0);
err:
	if( res )
		mysql_free_result(res);

	return fmt;
}


int	CLogParser::request_binlog_dump()
{
	unsigned char buf[1024];

	if( _binlog_name.empty() || !_binlog_pos )
		return -1;
	
	int4store(buf, _binlog_pos);
	int2store(buf + 4, 0); // flags
	int4store(buf + 6, _slave_id); 
	memcpy(buf + 10, _binlog_name.c_str(), _binlog_name.length());

	if( simple_command(&_mysql, COM_BINLOG_DUMP, (const unsigned char*)buf, _binlog_name.length() + 10, 1) )
	{
		on_error("COM_BINLOG_DUMP failed", &_mysql);
		return -1;
	}
	
	return 0;
}


void CLogParser::dispatch_events()
{
	unsigned long len;

	while (connect() != 0)
	{
		if( !_dispatch ) return;
		sleep(1);
	}
		
	if (build_db_structure() != 0 || request_binlog_dump() != 0)
	{
		disconnect();
		return;
	}

	CRotateLogEvent event_rotate;
	CQueryLogEvent event_query;
	CRowLogEvent event_row;
	CUnhandledLogEvent event_unhandled;
	uint32_t event_type;
	uint8_t *buf;
	
	CDatabase *db;
	CTable *tbl;
	do
	{
		len = cli_safe_read(&_mysql);
		if ( len != packet_error && len > 8 && (buf = _mysql.net.read_pos) != NULL && buf[0] != 254 )
		{
			buf++; len--;

			if (len < EVENT_LEN_OFFSET || buf[EVENT_TYPE_OFFSET] >= ENUM_END_EVENT || (uint32_t) len != uint4korr(buf+EVENT_LEN_OFFSET))
			{
				on_error("event sanity check failed", 0);
				break;
			}

			event_type = buf[EVENT_TYPE_OFFSET];
			if( event_type > _fmt->_number_of_event_types && event_type != FORMAT_DESCRIPTION_EVENT )
			{
				on_error("event not supported", 0);
				break;
			}
			
			switch( event_type )
			{
			case ROTATE_EVENT:
			{
				if (event_rotate.tune(buf, len, _fmt) == 0)
				{
					_binlog_name.assign((const char*)event_rotate.get_log_name(), event_rotate.get_log_name_len());
					_binlog_pos = event_rotate.get_log_pos();
					event_rotate.dump(stdout);
				}
				else
					on_error("rotate event tuning failed", NULL);
				break;
			}
			case FORMAT_DESCRIPTION_EVENT:
			{
				// ничего не делаем, главное, не выставить позицию
				break;
			}
					
			case QUERY_EVENT:
			{
				if (event_query.tune(buf, len, _fmt) == 0)
				{
					event_query.dump(stdout);
					_binlog_pos = event_query._log_pos;
				}
				break;
			}		
			case TABLE_MAP_EVENT:
			{
				db = (CDatabase*)find(CTableMapLogEvent::get_database_name(buf, len, _fmt));
				if (db != NULL) tbl = (CTable*)db->find(CTableMapLogEvent::get_table_name(buf, len, _fmt));
				if (tbl != NULL)
				{
					if( tbl->tune(buf, len, _fmt) != 0 )
					{
						on_error("tablemap event tuning failed", NULL);
						tbl = NULL;
					}
					else
						tbl->dump(stdout);
				}
				break;
			}
			case WRITE_ROWS_EVENT:
			case UPDATE_ROWS_EVENT:
			case DELETE_ROWS_EVENT:
			{
				// получаем таблицу 
				if( tbl )
				{
					if( event_row.tune(buf, len, _fmt) == 0 )
					{
						event_row.dump(stdout);
						tbl->update(event_row);
						if( event_type == WRITE_ROWS_EVENT )
							on_insert(*tbl, tbl->get_rows());
						else if( event_type == UPDATE_ROWS_EVENT ) 
							on_update(*tbl, tbl->get_new_rows(), tbl->get_rows());
						else 
							on_delete(*tbl, tbl->get_rows());
					}
					else
						on_error("rows event tuning failed", NULL);
				}
				break;
			}
			default:
			{
				if (event_unhandled.tune(buf, len, _fmt) == 0)
				{
					_binlog_pos = event_unhandled._log_pos;
					event_unhandled.dump(stdout);
				}
				else
					on_error("unhandled event tuning failed", NULL);
			}
			}
		}
		else
		{
			on_error("cli_safe_read error", &_mysql);
			
			while( _dispatch && reconnect() != 0 )
				sleep(1);
		}
	}
	while( _dispatch );

	disconnect();
}

void CLogParser::on_reconnect(MYSQL *mysql) {
	// stop_event_loop();
}

void CLogParser::on_data_incoming(MYSQL *mysql) {
	// stop_event_loop(); // при желании 
}

void CLogParser::stop_event_loop() 
{
	_dispatch = 0;	
}


void CLogParser::Dump(uint8_t *buf, size_t len)
{
	for(size_t i=0; i<len; i++)
		printf("%02X ", (unsigned int)buf[i]);
	printf("\n");
}



void CLogParser::on_error(const char *err, MYSQL *mysql)
{
	if( mysql )
		fprintf(stderr, "%s: %s\n", err, mysql_error(mysql));
	else
		fprintf(stderr, "%s\n", err);
}


int CLogParser::watch(const char* db_name, const char* table_name)
{
	if( !db_name && !table_name )
		return -1;
	
	std::string dbname(db_name);
	std::string tblname(table_name);
	
	CDatabase *db = (CDatabase*)find(&dbname);
	if (!db)
	{
		db = new CDatabase();
		_databases[dbname] = db;
	}
	db->watch(tblname);
	
	return 0;
}


int CLogParser::build_db_structure()
{
	int rc = -1;
	int atom_found;
	MYSQL_RES *res_db = NULL, *res_tbl = NULL, *res_column = NULL;
	MYSQL_ROW row;
	
	if( _databases.empty() )
	{
		on_error("build_db_structure() failed: no databases filtered", NULL);	
		return rc;
	}
	
	if( mysql_query(&_mysql, "show databases") || 
		!( res_db = mysql_store_result(&_mysql) ) 
	)
	{
		on_error("build_db_structure() call to 'show databases' failed", &_mysql);
		goto err;
	}
	
	atom_found = 0;
	CDatabase *db;
	CTable *tbl;
	char query[512];
	
	
	// TODO: check for tables and columns
	while( (row = mysql_fetch_row(res_db)) != NULL )
	{
		if( (db = (CDatabase*)this->find(row[0])) != NULL  )
		{
			if( mysql_select_db(&_mysql, row[0]) ||
				mysql_query(&_mysql, "show tables") ||
				!( res_tbl = mysql_store_result(&_mysql)))
			{
				on_error("build_db_structure() call to 'show tables' failed", &_mysql);
				goto err;
			}
			
			while( (row = mysql_fetch_row(res_tbl)) != NULL )
			{
				if( (tbl = (CTable*)db->find(row[0])) != NULL  )
				{
					sprintf(query, "desc %s", row[0]);
					if( mysql_query(&_mysql, query) ||
						!( res_column = mysql_store_result(&_mysql)))
					{
						on_error("build_db_structure() call to 'desc table' failed", &_mysql);
						goto err;
					}
					
					int pos=0;
					while( (row = mysql_fetch_row(res_column)) != NULL )
						tbl->watch(row[0], pos++);
					
					mysql_free_result(res_column);
					res_column = NULL;
				}
			}
			
			mysql_free_result(res_tbl);
			res_tbl = NULL;
		}
	}
	
	if( _binlog_name.empty() || !_binlog_pos )
		rc = get_last_binlog_position();
	
	rc = 0;
	
err:
	if (res_db)
		mysql_free_result(res_db);
	if (res_tbl)
		mysql_free_result(res_tbl);
	if (res_column)
		mysql_free_result(res_tbl);
	
	
	return rc;
}

int CLogParser::get_last_binlog_position()
{
	int rc = -1;
	MYSQL_RES* res;
	MYSQL_ROW row;
	
	if (mysql_query(&_mysql, "SHOW MASTER STATUS") || (res = mysql_store_result(&_mysql)) == NULL)
	{
		on_error("build_db_structure() failed while 'show master status'", &_mysql);
		goto err;
	}
	
	if ((row = mysql_fetch_row(res)) == NULL)
	{
		on_error("build_db_structure() failed: 'show master status' returns 0 rows", &_mysql);
		goto err;
	}
	
	if ( row[0] && row[0][0] && row[1] )
	{
		_binlog_name = row[0];
		_binlog_pos = (uint32_t)atoll(row[1]);
		printf("change binlog position to '%s:%d'\n", _binlog_name.c_str(), _binlog_pos);
	}
	else
		on_error("build_db_structure() failed: 'show master status' returns invalid row", &_mysql);
	
	rc = 0;
err:
	if (res)
		mysql_free_result(res);
	
	return rc;
}





}
