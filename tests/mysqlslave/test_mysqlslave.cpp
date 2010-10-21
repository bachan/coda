#include <stdio.h>
#include <iostream>
#include <vector>

#include <mysqlslave/logparser.h>




class sample_binlog_reader : public mysql::CLogParser
{
public:
	virtual int on_insert(const mysql::CTable &table, const mysql::CTable::TRows &newrows)
	{
		std::cout << "inserted " << newrows.size() << " rows into " << table.get_database_name() << "." << table.get_table_name() << std::endl;
		
		if( !strcasecmp(table.get_table_name(), "t2") ) dump_t2_rows(newrows);
		if( !strcasecmp(table.get_table_name(), "t3") ) dump_t3_rows(newrows);
		
		return 0;
		
	}
	virtual int on_update(const mysql::CTable &table, const mysql::CTable::TRows &newrows, const mysql::CTable::TRows &oldrows)
	{
		std::cout << "updated " << newrows.size() << " rows in " << table.get_database_name() << "." << table.get_table_name() << std::endl;
		
		if( !strcasecmp(table.get_table_name(), "t2") )
		{
			std::cout << "old rows" << std::endl; dump_t2_rows(oldrows);
			std::cout << "new rows" << std::endl; dump_t2_rows(newrows);
		}
		
		if( !strcasecmp(table.get_table_name(), "t3") )
		{
			std::cout << "old rows" << std::endl; dump_t3_rows(oldrows);
			std::cout << "new rows" << std::endl; dump_t3_rows(newrows);
		}
		
		return 0;
	}
	virtual int on_delete(const mysql::CTable &table, const mysql::CTable::TRows &rows)
	{
		std::cout << "deleted " << rows.size() << " rows from " 
		<< table.get_database_name() << "." << table.get_table_name() << std::endl;
		
		if( !strcasecmp(table.get_table_name(), "t2") ) dump_t2_rows(rows);
		if( !strcasecmp(table.get_table_name(), "t3") ) dump_t3_rows(rows);
		
		return 0;
	}
private:
	void dump_t2_rows(const mysql::CTable::TRows &rows)
	{
		for(mysql::CTable::TRows::const_iterator it = rows.begin(); it != rows.end(); ++it )
			std::cout << (*it)[0].as_int() << "\t" << (*it)[1].as_std_str() << std::endl;
	}

	void dump_t3_rows(const mysql::CTable::TRows &rows)
	{
		uint64_t id1, id2;
		std::string str;
		for(mysql::CTable::TRows::const_iterator it = rows.begin(); it != rows.end(); ++it )
		{
			const mysql::CRow &row = *it;
			id1 = row["id"].as_uint();
			id2 = row["t2_id"].as_uint();
			str = row[2].as_std_str();
			std::cout <<  id1 << "\t" << id2 << "\t" << str << std::endl;
		}
	}

};


class coda_binlog_reader : public mysql::CLogParser
{
public:
	virtual int on_insert(const mysql::CTable &table, const mysql::CTable::TRows &newrows)
	{
		return 0;
	}
	virtual int on_update(const mysql::CTable &table, const mysql::CTable::TRows &newrows, const mysql::CTable::TRows &oldrows)
	{
		return 0;
	}
	virtual int on_delete(const mysql::CTable &table, const mysql::CTable::TRows &rows)
	{
		return 0;
	}
};

#define MYSQL_HOST "192.168.3.101"
#define MYSQL_USER "testy"
#define MYSQL_PWD "testy"
#define MYSQL_BINLOG_NAME "mysql_binary_log.000002"
#define MYSQL_BINLOG_POS 106
#define LOCAL_BINLOG_READER_ID 1



int main() 
{
	sample_binlog_reader binlog_reader;
	binlog_reader.set_connection_params(MYSQL_HOST, LOCAL_BINLOG_READER_ID, MYSQL_USER, MYSQL_PWD);
	// будет позиционироваться при старте к последнему логу, если позиция не указана явно
	//binlog_reader.set_binlog_position(MYSQL_BINLOG_NAME,MYSQL_BINLOG_POS, 1);
	binlog_reader.watch("test", "t2");
	binlog_reader.watch("test", "t3");
	
	binlog_reader.dispatch_events();
	return 0;

}

// TODO: не сбивать позиции при реконнекте
