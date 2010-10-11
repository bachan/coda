#include <stdio.h>
#include <mysqlslave/logparser.h>

using namespace std;

int main() {

	const char *host	= "192.168.3.101";
	const char *user	= "testy";

	const char *pwd	= "testy";
	mysql::CLogParser p;

	p.set_connection_params(host, user, pwd);
	p.set_binlog_position("mysql_binary_log.000002",106, 1);

	p.connect();
//	p.request_dump("mysql_binary_log.000001",106, 1);

	p.dispatch_events();

	return 0;

}
