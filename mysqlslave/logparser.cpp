/*
 * logparser.cpp
 *
 *  Created on: 28.09.2010
 *      Author: azrahel
 */

#include "logparser.h"

namespace mysql {

CLogParser::CLogParser()
	: _port(0)
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

void CLogParser::connect()
{
	disconnect();
	if( !mysql_init(&_mysql) )
	{
		on_error("mysql_init failed", 0);
		return;
	}

	if( !mysql_real_connect(&_mysql, _host.c_str(), _user.c_str(), _passwd.c_str(), 0, _port, 0, 0) )
		on_error("mysql_real_connect() failed", &_mysql);
}

void CLogParser::reconnect()
{
	connect();
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


CFormatDescriptionLogEvent* CLogParser::get_binlog_format(MYSQL *mysql)
{
	MYSQL_RES* res = 0;
	MYSQL_ROW row;
	const char* version;
	CFormatDescriptionLogEvent* fmt = 0;


	if( mysql_query(mysql, "SELECT VERSION()") )
	{
		on_error("could not find server version", mysql);
		goto err;
	}

	if( !( res = mysql_store_result(mysql) ) )
	{
		on_error("could not find server version", mysql);
		goto err;
	}

	if( !( row = mysql_fetch_row(res) ) )
	{
		on_error("could not find server version", mysql);
		goto err;
	}

	if( !(version = row[0]) )
	{
		on_error("could not find server version", mysql);
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

int CLogParser::on_update_rows(CRowLogEvent *rlev)
{
	return 0;
}


int CLogParser::dispatch_events()
{
	CLogEvent *ev;

	unsigned long len;

	if( _fmt )
	{
		delete _fmt;
		_fmt = 0;
	}

	_fmt = get_binlog_format(&_mysql);
	if( !_fmt )
		return -1;

	request_binlog_dump(_binlog_name.c_str(), _binlog_pos, _server_id, _binlog_flags);

	CRowLogEvent rlev;
	uint32_t event_type;
	uint8_t *buf;
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
					return 0;
				}

				event_type = buf[EVENT_TYPE_OFFSET];
				if( event_type > _fmt->_number_of_event_types && event_type != FORMAT_DESCRIPTION_EVENT )
				{
					on_error("event not supported", 0);
					return 0;
				}

				CTableMapLogEvent *tmev;
				TTablesRepo::iterator it;

				switch( event_type )
				{
				case TABLE_MAP_EVENT:
				{
					uint64_t table_id = CTableMapLogEvent::get_table_id(buf, len, _fmt);

					it = _tables.find(table_id);
					if( it == _tables.end() )
					{
						fprintf(stdout, "new table_id %llu\n", (unsigned long long)table_id);
						tmev = new CTableMapLogEvent(buf, len, _fmt);
						_tables[table_id] = tmev;
					}
					else
					{
						fprintf(stdout, "found table_id %llu\n", (unsigned long long)table_id);
						tmev = it->second;
					}

					ev = tmev;
					break;
				}
				case WRITE_ROWS_EVENT:
				case UPDATE_ROWS_EVENT:
				case DELETE_ROWS_EVENT:
				{
					rlev.tune(buf, len, _fmt);
					if( rlev.is_valid() )
					{
						it = _tables.find(rlev._table_id);
						if( it != _tables.end() )
						{
							on_update_rows(&rlev);
						}

					}
					else
						on_error("invalid row log event", 0);
				}

				}


//				ev = build_event(_mysql->net.read_pos + 1, len - 1, _fmt);
//				fprintf(stdout,"%s\t", ev->get_type_code_str());
//				ev->dump(stdout);
//				fprintf(stdout, "\n");
			}
		}
		else
		{
			on_error("cli_safe_read error", &_mysql);
			_dispatch = 0;
		}
	}
	while( _dispatch );

//	fprintf(stderr, "read %u bytes, wow!\n", len);





	return 0;
}


CLogEvent* CLogParser::build_event(uint8_t *buf, size_t len, CFormatDescriptionLogEvent *fmt)
{
	CLogEvent *ev;
	uint32_t event_type;

	/* Check the integrity */
	if( len < EVENT_LEN_OFFSET ||
			buf[EVENT_TYPE_OFFSET] >= ENUM_END_EVENT ||
				(uint32_t) len != uint4korr(buf+EVENT_LEN_OFFSET))
	{
		on_error("event sanity check failed", 0);
		return 0;
	}

	event_type = buf[EVENT_TYPE_OFFSET];
	if( event_type > fmt->_number_of_event_types && event_type != FORMAT_DESCRIPTION_EVENT )
	{
		on_error("event not supported", 0);
		return 0;
	}


	switch( event_type )
	{
	case QUERY_EVENT:
		ev = new CQueryLogEvent(buf, len, fmt, QUERY_EVENT);
		break;
	case INTVAR_EVENT:
		ev = new CIntvarLogEvent(buf, len, fmt);
		break;
	case TABLE_MAP_EVENT:
	{
		CTableMapLogEvent *tmev;
		uint64_t table_id = CTableMapLogEvent::get_table_id(buf, len, fmt);
		TTablesRepo::iterator it;

		it = _tables.find(table_id);
		if( it == _tables.end() )
		{
			fprintf(stdout, "new table_id %llu\n", (unsigned long long)table_id);
			tmev = new CTableMapLogEvent(buf, len, fmt);
			_tables[table_id] = tmev;
		}
		else
		{
			fprintf(stdout, "found table_id %llu\n", (unsigned long long)table_id);
			tmev = it->second;
		}

		ev = tmev;
		break;
	}
	default:
		ev = new CUnknownLogEvent(buf, len, fmt);
	}

	return ev;
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







}
