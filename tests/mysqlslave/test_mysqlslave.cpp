#include <stdio.h>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <mysqlslave/logparser.h>

#include <tr1/unordered_map>

class coda_binlog_reader : public mysql::CLogParser
{
public:
	virtual int on_insert(mysql::CTable &table, mysql::CTable::TRows &newrows)
	{
		return 0;
	}
	virtual int on_update(mysql::CTable &table, mysql::CTable::TRows &newrows, mysql::CTable::TRows &oldrows)
	{
		return 0;
	}
	virtual int on_delete(mysql::CTable &table, mysql::CTable::TRows &newrows)
	{
		return 0;
	}
};

int main() 
{
	const int runtime = 600;
	const char *host = "192.168.3.101";
	const char *user = "testy";
	const char *pwd	= "testy";
	
	mysql::CContainer *db, *tbl;
	
	coda_binlog_reader binlog_reader;
	
	db = static_cast<mysql::CContainer*>(binlog_reader.watch("test"));
	db->watch("t3");
	db->watch("t2");
	
	binlog_reader.set_connection_params(host, user, pwd);
	binlog_reader.set_binlog_position("mysql_binary_log.000002",106, 1);
	binlog_reader.dispatch_events();
	return 0;

}
