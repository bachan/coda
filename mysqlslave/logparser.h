/*
 * logparser.h
 *
 *  Created on: 28.09.2010
 *      Author: azrahel
 */

#ifndef LOGPARSER_H_
#define LOGPARSER_H_

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string>

// #include <my_config.h>
// #include <my_global.h>
// #include <mysql.h>
// #include <m_ctype.h>
// #include <my_sys.h>
// #include <my_dir.h>
// #include <sql_common.h>
// #include <m_string.h>
// #include <mysqld_error.h>
// #include <my_attribute.h>
// #include <my_dbug.h>



#include "logevent.h"
#include "database.h"

#include <map>
namespace mysql {

class CLogParser : public CItem
{
public:
	typedef std::map<uint32_t, CTableMapLogEvent*> TTablesRepo;
public:
	CLogParser();
	virtual ~CLogParser() throw();

	
	void set_connection_params(const char *host, const char *user, const char *passwd, int port = 0);
	void set_binlog_position(const char *fname, uint32_t pos, uint32_t srv_id, uint16_t flags = 0);


	void dispatch_events();
	void stop_event_loop();

public:
	virtual CItem* watch(std::string name);
protected:
	virtual int on_insert(CTable &table, CTable::TRow &row)
	{
		return 0;
	}
	virtual int on_update(CTable &table, CTable::TRow &row, uint64_t update_mask)
	{
		return 0;
	}
	virtual int on_delete(CTable &table, CTable::TRow &row)
	{
		return 0;
	}
		
	
protected:
	virtual void on_data_incoming(MYSQL *mysql);
	virtual void on_reconnect(MYSQL *mysql);
	virtual void on_error(const char *err, MYSQL *mysql);
	
protected:
	int connect();
	int reconnect();
	void disconnect();
	CFormatDescriptionLogEvent* get_binlog_format();
	int request_binlog_dump(const char *fname, uint32_t pos, uint32_t srv_id, uint16_t flags = 0);
	void Dump(uint8_t *buf, size_t len);

protected:
	TItems &_databases;
	MYSQL _mysql;
	std::string _host;
	int _port;
	std::string _user;
	std::string _passwd;
	CFormatDescriptionLogEvent* _fmt;
	std::string _binlog_name;
	uint32_t _binlog_pos;
	uint32_t _binlog_flags;
	uint32_t _server_id;
private:
	const char* _err;
	volatile int _dispatch;
};



}

#endif /* LOGPARSER_H_ */
