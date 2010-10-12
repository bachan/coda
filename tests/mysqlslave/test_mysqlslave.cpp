#include <stdio.h>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <mysqlslave/logparser.h>


class coda_binlog_reader : public mysql::CLogParser
{
#if 0
public:
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
#endif
};

int main() {

	const char *host = "192.168.3.102";
	const char *user = "testy";
	const char *pwd	= "testy";
	
	mysql::CItem *db, *tbl;
	
	coda_binlog_reader binlog_reader;
	
	db = binlog_reader.watch("test");
		tbl = db->watch("t2");
			tbl->watch("id");
			tbl->watch("name");
	
	binlog_reader.set_connection_params(host, user, pwd);
	binlog_reader.set_binlog_position("mysql_binary_log.000002",106, 1);
	// binlog_reader.connect();
	// binlog_reader.dispatch_events();
	
	
	const int runtime = 10;
	
	std::cout << "starting dispatcher for " << runtime << " seconds" << std::endl;
	boost::thread dispatcher_thread( boost::bind( &coda_binlog_reader::dispatch_events, &binlog_reader) );
	::sleep(runtime);
	binlog_reader.stop_event_loop();
	std::cout << "wait for dispatcher... ";
	dispatcher_thread.join();
	std::cout << "done." << std::endl;
	return 0;

}
