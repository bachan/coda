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

#include <my_config.h>
#include <my_global.h>
#include <mysql.h>
#include <m_ctype.h>
#include <my_sys.h>
#include <my_dir.h>
#include <sql_common.h>
#include <m_string.h>
#include <mysqld_error.h>
#include <my_attribute.h>
#include <my_dbug.h>

#include "logevent.h"

#include <map>

namespace mysql {

class CLogParser
{
public:
	typedef std::map<uint32_t, CTableMapLogEvent*> TTablesRepo;
public:
	CLogParser();
	virtual ~CLogParser() throw();

	void set_connection_params(const char *host, const char *user, const char *passwd, int port = 0);
	void set_binlog_position(const char *fname, uint32_t pos, uint32_t srv_id, uint16_t flags = 0);

	void connect();
	void reconnect();
	void disconnect();

	int dispatch_events();

protected:
	int request_binlog_dump(const char *fname, uint32_t pos, uint32_t srv_id, uint16_t flags = 0);
	void Dump(uint8_t *buf, size_t len);
protected:
	virtual	 int on_update_rows(CRowLogEvent *rlev);

	virtual		CFormatDescriptionLogEvent* get_binlog_format(MYSQL *mysql);
	virtual		CLogEvent* build_event(uint8_t *buf, size_t len, CFormatDescriptionLogEvent *fmt);
	virtual		void on_error(const char *err, MYSQL *mysql);




protected:
	TTablesRepo _tables;
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
	int _dispatch;
};



}

#endif /* LOGPARSER_H_ */
