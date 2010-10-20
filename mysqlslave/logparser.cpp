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
	, _port(0)
	, _fmt(0)
	, _binlog_pos(0)
	, _binlog_flags(0)
	, _server_id(0)
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
	
	if( _fmt )
		delete _fmt;
	
	_fmt = get_binlog_format();
	
	return _fmt ? 0 : 1;
}

int CLogParser::reconnect()
{
	return connect();
}

void CLogParser::disconnect()
{
	mysql_close(&_mysql);
}


void CLogParser::set_connection_params(const char *host, const char *user, const char *passwd, int port)
{
	_host	= host;
	_user	= user;
	_passwd	= passwd;
	_port	= port;
}

void CLogParser::set_binlog_position(const char *fname, uint32_t pos, uint32_t srv_id, uint16_t flags)
{
	_binlog_name = fname;
	_binlog_pos	= pos;
	_server_id = srv_id;
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




int	CLogParser::request_binlog_dump(const char *fname, uint32_t pos, uint32_t srv_id, uint16_t flags)
{
	int len;
	unsigned char buf[1024];

	if( !fname )
		return -1;

	len = strlen(fname);
	if( !len )
		return -1;

	int4store(buf, pos);
	int2store(buf + 4, flags);
	int4store(buf + 6, srv_id);
	memcpy(buf + 10, fname, len);

	if( simple_command(&_mysql, COM_BINLOG_DUMP, (const unsigned char*)buf, len + 10, 1) )
	{
		on_error("COM_BINLOG_DUMP", &_mysql);
		return -1;
	}

	return 0;
}


void CLogParser::dispatch_events()
{
	unsigned long len;

	while( connect() != 0 )
	{
		if( !_dispatch )
			return;
		sleep(1);
	}
		
	if(	!build_db_structure() )
		return;
	// пока что так, потом будем позиционироваться автоматически
	request_binlog_dump(_binlog_name.c_str(), _binlog_pos, _server_id, _binlog_flags);

	CRowLogEvent event_row;
	CQueryLogEvent event_query;
	CUnknownLogEvent event_unknown;
	uint32_t event_type;
	uint8_t *buf;
	
	CDatabase *db;
	CTable *tbl;
	do
	{
		len = cli_safe_read(&_mysql);
		if( len != packet_error )
		{
			buf = _mysql.net.read_pos;
			if( (_dispatch = !(len < 8 && buf[0] == 254)) != 0 )
			{
				buf++; len--;

				if( len < EVENT_LEN_OFFSET ||
						buf[EVENT_TYPE_OFFSET] >= ENUM_END_EVENT ||
							(uint32_t) len != uint4korr(buf+EVENT_LEN_OFFSET))
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

				TTablesRepo::iterator it;

				switch( event_type )
				{
				case QUERY_EVENT:
				{
					event_query.tune(buf, len, _fmt);
					//event_query.dump(stdout);
					break;
				}		
				case TABLE_MAP_EVENT:
				{
/*					printf("db: %s\ttable: %s\t table_id: %d\n",
						CTableMapLogEvent::get_database_name(buf, len, _fmt),
						CTableMapLogEvent::get_table_name(buf, len, _fmt),
						(int)CTableMapLogEvent::get_table_id(buf, len, _fmt));*/
					db = static_cast<CDatabase*>(this->find(CTableMapLogEvent::get_database_name(buf, len, _fmt)));
					if( db )
						tbl = static_cast<CTable*>(db->find(CTableMapLogEvent::get_table_name(buf, len, _fmt)));
					if( tbl && tbl->tune(buf, len, _fmt) != 0 )
						tbl = NULL;
					break;
					
					
				}
				case WRITE_ROWS_EVENT:
				case UPDATE_ROWS_EVENT:
				case DELETE_ROWS_EVENT:
				{
					if( tbl )
					{
						if( event_row.tune(buf, len, _fmt) == 0 )
						{
//							event_row.dump(stdout);
							tbl->update(event_row);
							if( event_type == WRITE_ROWS_EVENT )
								on_insert(*tbl, tbl->get_rows());
							else if( event_type == UPDATE_ROWS_EVENT ) 
								on_update(*tbl, tbl->get_new_rows(), tbl->get_rows());
							else 
								on_delete(*tbl, tbl->get_rows());
						}
						else
							on_error("event_row tuning failed", NULL);
					}
					break;
				}
				default:
				{
					;
/*					event_unknown.tune(buf, len, _fmt);
					event_unknown.dump(stdout);*/
				}
				}
			}
		}
		else
		{
			on_error("cli_safe_read error", &_mysql);
			on_reconnect(&_mysql);
			
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


IItem* CLogParser::watch(std::string name)
{
	IItem *db = find(&name);
	if( !db )
	{
		db = new CDatabase();
		_databases[name] = db;
	}
	return db;
}


int CLogParser::build_db_structure()
{
	int rc = 0;
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
	
	rc = 1;
	
err:
	if( res_db )
		mysql_free_result(res_db);
	if( res_tbl )
		mysql_free_result(res_tbl);
	if( res_column )
		mysql_free_result(res_tbl);
	
	return rc;
}





}
