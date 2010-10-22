/*
 * logevent.h
 *
 *  Created on: 28.09.2010
 *      Author: azrahel
 */

#ifndef LOGEVENT_H_
#define LOGEVENT_H_

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

// -- mysql developers are gays begin
#undef max
#undef min
// -- mysql developers are gays end
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define LOG_READ_EOF    -1
#define LOG_READ_BOGUS  -2
#define LOG_READ_IO     -3
#define LOG_READ_MEM    -5
#define LOG_READ_TRUNC  -6
#define LOG_READ_TOO_LARGE -7

#define LOG_EVENT_OFFSET 4


#define BINLOG_VERSION    4
#define ST_SERVER_VER_LEN 50



#define DUMPFILE_FLAG		0x1
#define OPT_ENCLOSED_FLAG	0x2
#define REPLACE_FLAG		0x4
#define IGNORE_FLAG		0x8

#define FIELD_TERM_EMPTY	0x1
#define ENCLOSED_EMPTY		0x2
#define LINE_TERM_EMPTY		0x4
#define LINE_START_EMPTY	0x8
#define ESCAPED_EMPTY		0x10




#define NUM_LOAD_DELIM_STRS 5


#define LOG_EVENT_HEADER_LEN 19     /* the fixed header length */
#define OLD_HEADER_LEN       13     /* the fixed header length in 3.23 */


#define LOG_EVENT_MINIMAL_HEADER_LEN 19

/* event-specific post-header sizes */
// where 3.23, 4.x and 5.0 agree
#define QUERY_HEADER_MINIMAL_LEN     (4 + 4 + 1 + 2)
// where 5.0 differs: 2 for len of N-bytes vars.
#define QUERY_HEADER_LEN     (QUERY_HEADER_MINIMAL_LEN + 2)
#define STOP_HEADER_LEN      0
#define LOAD_HEADER_LEN      (4 + 4 + 4 + 1 +1 + 4)
#define SLAVE_HEADER_LEN     0
#define START_V3_HEADER_LEN     (2 + ST_SERVER_VER_LEN + 4)
#define ROTATE_HEADER_LEN    8 // this is FROZEN (the Rotate post-header is frozen)
#define INTVAR_HEADER_LEN      0
#define CREATE_FILE_HEADER_LEN 4
#define APPEND_BLOCK_HEADER_LEN 4
#define EXEC_LOAD_HEADER_LEN   4
#define DELETE_FILE_HEADER_LEN 4
#define NEW_LOAD_HEADER_LEN    LOAD_HEADER_LEN
#define RAND_HEADER_LEN        0
#define USER_VAR_HEADER_LEN    0
#define FORMAT_DESCRIPTION_HEADER_LEN (START_V3_HEADER_LEN+1+LOG_EVENT_TYPES)
#define XID_HEADER_LEN         0
#define BEGIN_LOAD_QUERY_HEADER_LEN APPEND_BLOCK_HEADER_LEN
#define ROWS_HEADER_LEN        8
#define TABLE_MAP_HEADER_LEN   8
#define EXECUTE_LOAD_QUERY_EXTRA_HEADER_LEN (4 + 4 + 4 + 1)
#define EXECUTE_LOAD_QUERY_HEADER_LEN  (QUERY_HEADER_LEN + EXECUTE_LOAD_QUERY_EXTRA_HEADER_LEN)
#define INCIDENT_HEADER_LEN    2


/*
  Max number of possible extra bytes in a replication event compared to a
  packet (i.e. a query) sent from client to master;
  First, an auxiliary log_event status vars estimation:
*/
#define MAX_SIZE_LOG_EVENT_STATUS (1 + 4          /* type, flags2 */   + \
                                   1 + 8          /* type, sql_mode */ + \
                                   1 + 1 + 255    /* type, length, catalog */ + \
                                   1 + 4          /* type, auto_increment */ + \
                                   1 + 6          /* type, charset */ + \
                                   1 + 1 + 255    /* type, length, time_zone */ + \
                                   1 + 2          /* type, lc_time_names_number */ + \
                                   1 + 2          /* type, charset_database_number */ + \
                                   1 + 8          /* type, table_map_for_update */ + \
                                   1 + 4          /* type, master_data_written */ + \
                                   1 + 16 + 1 + 60/* type, user_len, user, host_len, host */)
#define MAX_LOG_EVENT_HEADER   ( /* in order of Query_log_event::write */ \
  LOG_EVENT_HEADER_LEN + /* write_header */ \
  QUERY_HEADER_LEN     + /* write_data */   \
  EXECUTE_LOAD_QUERY_EXTRA_HEADER_LEN + /*write_post_header_for_derived */ \
  MAX_SIZE_LOG_EVENT_STATUS + /* status */ \
  NAME_LEN + 1)

/*
   Event header offsets;
   these point to places inside the fixed header.
*/

#define EVENT_TYPE_OFFSET    4
#define SERVER_ID_OFFSET     5
#define EVENT_LEN_OFFSET     9
#define LOG_POS_OFFSET       13
#define FLAGS_OFFSET         17

/* start event post-header (for v3 and v4) */

#define ST_BINLOG_VER_OFFSET  0
#define ST_SERVER_VER_OFFSET  2
#define ST_CREATED_OFFSET     (ST_SERVER_VER_OFFSET + ST_SERVER_VER_LEN)
#define ST_COMMON_HEADER_LEN_OFFSET (ST_CREATED_OFFSET + 4)

/* slave event post-header (this event is never written) */

#define SL_MASTER_PORT_OFFSET   8
#define SL_MASTER_POS_OFFSET    0
#define SL_MASTER_HOST_OFFSET   10

/* query event post-header */

#define Q_THREAD_ID_OFFSET	0
#define Q_EXEC_TIME_OFFSET	4
#define Q_DB_LEN_OFFSET		8
#define Q_ERR_CODE_OFFSET	9
#define Q_STATUS_VARS_LEN_OFFSET 11
#define Q_DATA_OFFSET		QUERY_HEADER_LEN
/* these are codes, not offsets; not more than 256 values (1 byte). */
#define Q_FLAGS2_CODE           0
#define Q_SQL_MODE_CODE         1
/*
  Q_CATALOG_CODE is catalog with end zero stored; it is used only by MySQL
  5.0.x where 0<=x<=3. We have to keep it to be able to replicate these
  old masters.
*/
#define Q_CATALOG_CODE          2
#define Q_AUTO_INCREMENT	3
#define Q_CHARSET_CODE          4
#define Q_TIME_ZONE_CODE        5
/*
  Q_CATALOG_NZ_CODE is catalog withOUT end zero stored; it is used by MySQL
  5.0.x where x>=4. Saves one byte in every Query_log_event in binlog,
  compared to Q_CATALOG_CODE. The reason we didn't simply re-use
  Q_CATALOG_CODE is that then a 5.0.3 slave of this 5.0.x (x>=4) master would
  crash (segfault etc) because it would expect a 0 when there is none.
*/
#define Q_CATALOG_NZ_CODE       6

#define Q_LC_TIME_NAMES_CODE    7

#define Q_CHARSET_DATABASE_CODE 8

#define Q_TABLE_MAP_FOR_UPDATE_CODE 9

#define Q_MASTER_DATA_WRITTEN_CODE 10

#define Q_INVOKER 11

/* Intvar event post-header */

/* Intvar event data */
#define I_TYPE_OFFSET        0
#define I_VAL_OFFSET         1

/* Rand event data */
#define RAND_SEED1_OFFSET 0
#define RAND_SEED2_OFFSET 8

/* User_var event data */
#define UV_VAL_LEN_SIZE        4
#define UV_VAL_IS_NULL         1
#define UV_VAL_TYPE_SIZE       1
#define UV_NAME_LEN_SIZE       4
#define UV_CHARSET_NUMBER_SIZE 4

/* Load event post-header */
#define L_THREAD_ID_OFFSET   0
#define L_EXEC_TIME_OFFSET   4
#define L_SKIP_LINES_OFFSET  8
#define L_TBL_LEN_OFFSET     12
#define L_DB_LEN_OFFSET      13
#define L_NUM_FIELDS_OFFSET  14
#define L_SQL_EX_OFFSET      18
#define L_DATA_OFFSET        LOAD_HEADER_LEN

/* Rotate event post-header */
#define R_POS_OFFSET       0
#define R_IDENT_OFFSET     8

/* CF to DF handle LOAD DATA INFILE */

/* CF = "Create File" */
#define CF_FILE_ID_OFFSET  0
#define CF_DATA_OFFSET     CREATE_FILE_HEADER_LEN

/* AB = "Append Block" */
#define AB_FILE_ID_OFFSET  0
#define AB_DATA_OFFSET     APPEND_BLOCK_HEADER_LEN

/* EL = "Execute Load" */
#define EL_FILE_ID_OFFSET  0

/* DF = "Delete File" */
#define DF_FILE_ID_OFFSET  0

/* TM = "Table Map" */
#define TM_MAPID_OFFSET    0
#define TM_FLAGS_OFFSET    6

/* RW = "RoWs" */
#define RW_MAPID_OFFSET    0
#define RW_FLAGS_OFFSET    6

/* ELQ = "Execute Load Query" */
#define ELQ_FILE_ID_OFFSET QUERY_HEADER_LEN
#define ELQ_FN_POS_START_OFFSET ELQ_FILE_ID_OFFSET + 4
#define ELQ_FN_POS_END_OFFSET ELQ_FILE_ID_OFFSET + 8
#define ELQ_DUP_HANDLING_OFFSET ELQ_FILE_ID_OFFSET + 12

/* 4 bytes which all binlogs should begin with */
#define BINLOG_MAGIC        "\xfe\x62\x69\x6e"

/*
  The 2 flags below were useless :
  - the first one was never set
  - the second one was set in all Rotate events on the master, but not used for
  anything useful.
  So they are now removed and their place may later be reused for other
  flags. Then one must remember that Rotate events in 4.x have
  LOG_EVENT_FORCED_ROTATE_F set, so one should not rely on the value of the
  replacing flag when reading a Rotate event.
  I keep the defines here just to remember what they were.
*/
#ifdef TO_BE_REMOVED
#define LOG_EVENT_TIME_F            0x1
#define LOG_EVENT_FORCED_ROTATE_F   0x2
#endif

/*
   This flag only makes sense for Format_description_log_event. It is set
   when the event is written, and *reset* when a binlog file is
   closed (yes, it's the only case when MySQL modifies already written
   part of binlog).  Thus it is a reliable indicator that binlog was
   closed correctly.  (Stop_log_event is not enough, there's always a
   small chance that mysqld crashes in the middle of insert and end of
   the binlog would look like a Stop_log_event).

   This flag is used to detect a restart after a crash, and to provide
   "unbreakable" binlog. The problem is that on a crash storage engines
   rollback automatically, while binlog does not.  To solve this we use this
   flag and automatically append ROLLBACK to every non-closed binlog (append
   virtually, on reading, file itself is not changed). If this flag is found,
   mysqlbinlog simply prints "ROLLBACK" Replication master does not abort on
   binlog corruption, but takes it as EOF, and replication slave forces a
   rollback in this case.

   Note, that old binlogs does not have this flag set, so we get a
   a backward-compatible behaviour.
*/

#define LOG_EVENT_BINLOG_IN_USE_F       0x1

/**
  @def LOG_EVENT_THREAD_SPECIFIC_F

  If the query depends on the thread (for example: TEMPORARY TABLE).
  Currently this is used by mysqlbinlog to know it must print
  SET @@PSEUDO_THREAD_ID=xx; before the query (it would not hurt to print it
  for every query but this would be slow).
*/
#define LOG_EVENT_THREAD_SPECIFIC_F 0x4

/**
  @def LOG_EVENT_SUPPRESS_USE_F

  Suppress the generation of 'USE' statements before the actual
  statement. This flag should be set for any events that does not need
  the current database set to function correctly. Most notable cases
  are 'CREATE DATABASE' and 'DROP DATABASE'.

  This flags should only be used in exceptional circumstances, since
  it introduce a significant change in behaviour regarding the
  replication logic together with the flags --binlog-do-db and
  --replicated-do-db.
 */
#define LOG_EVENT_SUPPRESS_USE_F    0x8

/*
  Note: this is a place holder for the flag
  LOG_EVENT_UPDATE_TABLE_MAP_VERSION_F (0x10), which is not used any
  more, please do not reused this value for other flags.
 */

/**
   @def LOG_EVENT_ARTIFICIAL_F

   Artificial events are created arbitarily and not written to binary
   log

   These events should not update the master log position when slave
   SQL thread executes them.
*/
#define LOG_EVENT_ARTIFICIAL_F 0x20

/**
   @def LOG_EVENT_RELAY_LOG_F

   Events with this flag set are created by slave IO thread and written
   to relay log
*/
#define LOG_EVENT_RELAY_LOG_F 0x40



namespace mysql {

/**
  @enum Log_event_type

  Enumeration type for the different types of log events.
*/
enum Log_event_type
{
	/*
	Every time you update this enum (when you add a type), you have to
	fix Format_description_log_event::Format_description_log_event().
	*/
	UNKNOWN_EVENT= 0,
	START_EVENT_V3= 1,
	QUERY_EVENT= 2,
	STOP_EVENT= 3,
	ROTATE_EVENT= 4,
	INTVAR_EVENT= 5,
	LOAD_EVENT= 6,
	SLAVE_EVENT= 7,
	CREATE_FILE_EVENT= 8,
	APPEND_BLOCK_EVENT= 9,
	EXEC_LOAD_EVENT= 10,
	DELETE_FILE_EVENT= 11,
	/*
	NEW_LOAD_EVENT is like LOAD_EVENT except that it has a longer
	sql_ex, allowing multibyte TERMINATED BY etc; both types share the
	same class (Load_log_event)
	*/
	NEW_LOAD_EVENT= 12,
	RAND_EVENT= 13,
	USER_VAR_EVENT= 14,
	FORMAT_DESCRIPTION_EVENT= 15,
	XID_EVENT= 16,
	BEGIN_LOAD_QUERY_EVENT= 17,
	EXECUTE_LOAD_QUERY_EVENT= 18,

	TABLE_MAP_EVENT = 19,

	/*
	These event numbers were used for 5.1.0 to 5.1.15 and are
	therefore obsolete.
	*/
	PRE_GA_WRITE_ROWS_EVENT = 20,
	PRE_GA_UPDATE_ROWS_EVENT = 21,
	PRE_GA_DELETE_ROWS_EVENT = 22,

	/*
	These event numbers are used from 5.1.16 and forward
	*/
	WRITE_ROWS_EVENT = 23,
	UPDATE_ROWS_EVENT = 24,
	DELETE_ROWS_EVENT = 25,

	/*
	Something out of the ordinary happened on the master
	*/
	INCIDENT_EVENT= 26,

	/*
	Add new events here - right above this comment!
	Existing events (except ENUM_END_EVENT) should never change their numbers
	*/

	ENUM_END_EVENT /* end marker */
};

/*
   The number of types we handle in Format_description_log_event (UNKNOWN_EVENT
   is not to be handled, it does not exist in binlogs, it does not have a
   format).
*/
#define LOG_EVENT_TYPES (ENUM_END_EVENT-1)

enum Int_event_type
{
  INVALID_INT_EVENT = 0, LAST_INSERT_ID_EVENT = 1, INSERT_ID_EVENT = 2
};



/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CLogEvent
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

class CFormatDescriptionLogEvent;

class CLogEvent
{
public:
	CLogEvent();
	virtual ~CLogEvent() throw();

	int Info(uint8_t* buf, size_t event_len);

	virtual Log_event_type get_type_code() const = 0;
	virtual const char* get_type_code_str() const = 0;

	virtual bool is_valid() const = 0;
	virtual void dump(FILE *stream) const {
		char buf[64];
		char *when = ctime_r(&_when, buf);
		when[strlen(buf)-1] = '\0';
		//char *ctime_r(const time_t *timep, char *buf);
		fprintf(stream, "#%d %s: %s, log pos %llu\n", 
				get_type_code(), get_type_code_str(), when, (unsigned long long)_log_pos);
	}
	
	virtual int tune(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt);

	
public:
	/*
	The offset in the log where this event originally appeared (it is
	preserved in relay logs, making SHOW SLAVE STATUS able to print
	coordinates of the event in the master's binlog). Note: when a
	transaction is written by the master to its binlog (wrapped in
	BEGIN/COMMIT) the log_pos of all the queries it contains is the
	one of the BEGIN (this way, when one does SHOW SLAVE STATUS it
	sees the offset of the BEGIN, which is logical as rollback may
	occur), except the COMMIT query which has its real offset.
	*/
	uint64_t	_log_pos;
	/*
	Timestamp on the master(for debugging and replication of
	NOW()/TIMESTAMP).  It is important for queries and LOAD DATA
	INFILE. This is set at the event's creation time, except for Query
	and Load (et al.) events where this is set at the query's
	execution time, which guarantees good replication (otherwise, we
	could have a query and its event with different timestamps).
	*/
	time_t _when;

	/* Number of bytes written by write() function */
	uint32_t _data_written;

	/*
	The master's server id (is preserved in the relay log; used to
	prevent from infinite loops in circular replication).
	*/
	uint32_t _server_id;

	/**
	Some 16 flags. See the definitions above for LOG_EVENT_TIME_F,
	LOG_EVENT_FORCED_ROTATE_F, LOG_EVENT_THREAD_SPECIFIC_F, and
	LOG_EVENT_SUPPRESS_USE_F for notes.
	*/
	uint16_t _flags;

	/**
	A storage to cache the global system variable's value.
	Handling of a separate event will be governed its member.
	*/
	uint32_t _slave_exec_mode;


};


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CFormatDescriptionLogEvent
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
class CFormatDescriptionLogEvent : public CLogEvent
{
public:
	CFormatDescriptionLogEvent(uint8_t binlog_ver, const char* server_ver = NULL);
	virtual ~CFormatDescriptionLogEvent() throw();

	virtual Log_event_type get_type_code() const {
		return FORMAT_DESCRIPTION_EVENT;
	}
	virtual const char* get_type_code_str() const {
		return "format description event";
	}

	virtual bool is_valid() const {
		return _binlog_version == 4 && _post_header_len &&
				_common_header_len >= LOG_EVENT_MINIMAL_HEADER_LEN;
	}

	void calc_server_version_split();

public:
	time_t _created;
	uint16 _binlog_version;
	char _server_version[ST_SERVER_VER_LEN];
	uint8_t _common_header_len;
	uint8_t _number_of_event_types;

	/* The list of post-headers' lengthes */
	uint8_t *_post_header_len;
	uint8_t _server_version_split[3];
	const uint8_t *_event_type_permutation;


};

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CRotateLogEvent
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
class CRotateLogEvent : public CLogEvent
{
public:
	CRotateLogEvent() : _position(0), _new_log(NULL) 
	{
	}
	virtual Log_event_type get_type_code() const { return ROTATE_EVENT; };
	virtual const char* get_type_code_str() const { return "rotate event"; };
	virtual bool is_valid() const {return true;}
	
	virtual int tune(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt);
	
	const uint8_t* get_log_name(size_t *len) const { if (len) *len=_len; return _new_log; }
	const uint8_t* get_log_name() const { return _new_log; }
	size_t get_log_name_len() const { return _len; }
	uint64_t get_log_pos() const { return _position; }
	
	
	virtual void dump(FILE *stream) const {
		CLogEvent::dump(stream);
		char buf[256];
		memcpy(buf, _new_log, _len);
		buf[_len] = '\0';
		fprintf(stream, "%s:%llu\n", buf, (unsigned long long)_position);
	}
	
	
protected:
	uint64_t _position;
	uint8_t* _new_log;
	size_t _len;
	
};


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CUnhandledLogEvent
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

class CUnhandledLogEvent : public CLogEvent
{
public:
	virtual Log_event_type get_type_code() const {
		return (Log_event_type)_event_number;
	}
	virtual const char* get_type_code_str() const {
		return "unhandled event";
	}

	virtual bool is_valid() const {
		return 1;
	}

	virtual int tune(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt) 
	{
		CLogEvent::tune(data, size, fmt);
		_event_number = data[EVENT_TYPE_OFFSET];
		return 0;
	}

protected:
	uint8_t	_event_number;
};



/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CIntvarLogEvent
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

class CIntvarLogEvent : public CLogEvent
{
public:
	virtual Log_event_type get_type_code() const {
		return INTVAR_EVENT;
	}
	virtual const char* get_type_code_str() const {
		if( _type == LAST_INSERT_ID_EVENT )
			return "intvar event LAST_INSERT_ID";
		else if( _type == INSERT_ID_EVENT )
			return "intvar event INSERT_ID";
		else
			return "intvar event unknown type";
	}

	virtual bool is_valid() const {
		return 1;
	}

	virtual int tune(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt) ;
	

protected:
	uint8_t		_type;
	uint64_t	_val;

};


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CQueryLogEvent
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

class CQueryLogEvent : public CLogEvent
{
public:
	virtual Log_event_type get_type_code() const {
		return QUERY_EVENT;
	}
	virtual const char* get_type_code_str() const {
		return "query event";
	}
	virtual bool is_valid() const {
		return 1;
	}
	virtual void dump(FILE *stream) const {
		CLogEvent::dump(stream);
		fprintf(stream, "query '%s' with error_code %d, exec time: %ds\n", _query, (int)_error_code, (int)_q_exec_time);
	}
	
	virtual int tune(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt);
	
public:
	uint32_t _q_exec_time;
	uint32_t _q_len;
	uint32_t _db_len;
	uint16_t _error_code;
	uint16_t _status_vars_len;
	char _query[1024];


};



/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CTableMapLogEvent
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

class CTableMapLogEvent : public CLogEvent
{
public:
    CTableMapLogEvent();
//	CTableMapLogEvent(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt);
	virtual ~CTableMapLogEvent() throw();

	static const char* get_database_name(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt);
	static const char* get_table_name(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt);
	static uint64_t	get_table_id(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt);
	
	virtual Log_event_type get_type_code() const {
		return TABLE_MAP_EVENT;
	}
	virtual const char* get_type_code_str() const {
		return "table map event";
	}
	virtual bool is_valid() const {
		return 
		_db_name[0] && 
		_table_name[0] && 
		_column_count && 
		_column_types &&
		_metadata_length &&
		_metadata;
	}
	
	
	virtual int tune(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt);
	
	const char * get_database_name() const;
	const char * get_table_name() const;
	int get_column_count() const;

public:
	uint64_t _table_id;
protected:
	uint8_t *_data;
	size_t _size;
	char _db_name[255];
	char _table_name[255];
	uint64_t _column_count;
	uint8_t *_column_types;
	uint64_t _metadata_length;
	uint8_t *_metadata;
	uint64_t _bit_null_fields;
	
};


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CRowLogEvent
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
class CRowLogEvent : public CLogEvent
{
public:
	CRowLogEvent() : _valid(0) {
	}
	
	const uint8_t* rows_data() const {
		return _data;
	}
	const size_t rows_len() const {
		return _len;
	}
	
	virtual Log_event_type get_type_code() const {
		return (Log_event_type)_type;
	}
	virtual const char* get_type_code_str() const {
		if( _type == WRITE_ROWS_EVENT )
			return "WRITE_ROWS_EVENT";
		else if( _type == DELETE_ROWS_EVENT )
			return "DELETE_ROWS_EVENT";
		else if( _type == UPDATE_ROWS_EVENT )
			return "UPDATE_ROWS_EVENT";
		else
			return "unknown rows event";
	}
	virtual bool is_valid() const {
		return _valid;
	}
	virtual int is_field_set(int column, const uint8_t **value, size_t *len) {
		return 0;
	}
	virtual void dump(FILE *stream) const {
		CLogEvent::dump(stream);
		fprintf(stream, 
				"valid: %d, rowslen %d, rowflags %d, table_id %d, ncolumns %d, ucm %X (%d bits)",
				_valid, 
				(int)_len,
				(int)_row_flags,
				(int)_table_id, 
				(int)_ncolumns, 
				(int)get_used_columns_mask(),
				(int)get_used_columns_1bit_count());
		if( _type == UPDATE_ROWS_EVENT )
			fprintf(stream, ", ucam %X (%d bits)", 
				(int)get_used_columns_afterimage_mask(),
				(int)get_used_columns_afterimage_1bit_count());
		fprintf(stream, "\n");
	}
	
	
	virtual int tune(uint8_t *data, size_t size, const CFormatDescriptionLogEvent *fmt);
	
	uint64_t get_used_columns_mask() const {
		return _used_columns_mask & 0x00FFFFFFFFFFFFFFFF;
	}
	uint64_t get_used_columns_1bit_count() const {
		return _used_columns_mask >> (64-8);
	}
	
	uint64_t get_used_columns_afterimage_mask() const {
		return _used_columns_afterimage_mask & 0x00FFFFFFFFFFFFFFFF;
	}
	uint64_t get_used_columns_afterimage_1bit_count() const {
		return _used_columns_afterimage_mask >> (64-8);
	}
	
	uint64_t build_column_mask(const uint8_t **ptr, size_t *len, uint64_t n);
	void update_n1bits(uint64_t *mask);
	
public:
	uint32_t _type;
	uint16_t _row_flags;
	uint64_t _table_id;
	uint64_t _ncolumns;
	
	const uint8_t *_rows;
protected:
	// в самый старший байт пишем число установленных бит
	uint64_t _used_columns_mask;
	// в самый старший байт пишем число установленных бит
	uint64_t _used_columns_afterimage_mask; // for UPDATE_ROWS_EVENT only
	const uint8_t *_data;
	size_t _len;
	int _valid;

};


}

#endif /* LOGEVENT_H_ */
