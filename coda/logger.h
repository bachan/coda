#ifndef __CODA_LOGGER_H__
#define __CODA_LOGGER_H__

#include "helper_file.h"
#include "helper_time.h"

#define LOG_ACCESS  0  /* ACC: special level for access log      */
#define LOG_EMERG   1  /* EME: system is unusable                */
#define LOG_ALERT   2  /* ALE: action must be taken immediately  */
#define LOG_CRIT    3  /* CRI: critical conditions               */
#define LOG_ERROR   4  /* ERR: error conditions                  */
#define LOG_WARN    5  /* WRN: warning conditions                */
#define LOG_NOTICE  6  /* NOT: normal, but significant condition */
#define LOG_INFO    7  /* NFO: informational message             */

#define log_access(fmt,...) log_format(STDERR_FILENO,LOG_ACCESS,"[access]",fmt,##__VA_ARGS__)
#define log_emerg( fmt,...) log_format(STDERR_FILENO,LOG_EMERG, " [emerg]",fmt,##__VA_ARGS__)
#define log_alert( fmt,...) log_format(STDERR_FILENO,LOG_ALERT, " [alert]",fmt,##__VA_ARGS__)
#define log_crit(  fmt,...) log_format(STDERR_FILENO,LOG_CRIT,  "  [crit]",fmt,##__VA_ARGS__)
#define log_error( fmt,...) log_format(STDERR_FILENO,LOG_ERROR, " [error]",fmt,##__VA_ARGS__)
#define log_warn(  fmt,...) log_format(STDERR_FILENO,LOG_WARN,  "  [warn]",fmt,##__VA_ARGS__)
#define log_notice(fmt,...) log_format(STDERR_FILENO,LOG_NOTICE,"[notice]",fmt,##__VA_ARGS__)
#define log_info(  fmt,...) log_format(STDERR_FILENO,LOG_INFO,  "  [info]",fmt,##__VA_ARGS__)

#define LOG_FORMAT(lvstr,tname,fmt) "%04u/%02u/%02u/%02u:%02u:%02u "lvstr" "tname" "fmt"\n"
#define LOG_VALUES(tmgmt,tname,...) tmgmt.tm_year, tmgmt.tm_mon, tmgmt.tm_mday, \
    tmgmt.tm_hour, tmgmt.tm_min, tmgmt.tm_sec, tname, ##__VA_ARGS__

#define log_format(fd,level,lvstr,fmt,...) do {                             \
                                                                            \
    if (level <= log_level)                                                 \
    {                                                                       \
        struct tm tmgmt;                                                    \
        const char* tname;                                                  \
                                                                            \
        rdv_gmtime(time(NULL), &tmgmt);                                     \
                                                                            \
        if (NULL != (tname = log_thread_name_get()))                        \
        {                                                                   \
            dprintf(fd, LOG_FORMAT(lvstr,"%s:",fmt),                        \
                        LOG_VALUES(tmgmt,tname,##__VA_ARGS__));             \
        }                                                                   \
        else                                                                \
        {                                                                   \
            dprintf(fd, LOG_FORMAT(lvstr,"%u:",fmt),                        \
                        LOG_VALUES(tmgmt,pthread_self(),##__VA_ARGS__));    \
        }                                                                   \
    }                                                                       \
                                                                            \
} while (0)

extern volatile int log_level;
int log_levels(const char* level);
int log_create(const char* path, int level);
int log_thread_name_set(const char* name);
const char* log_thread_name_get();

#define log_create_from_str(path, level) log_create(path, log_levels(level))
#define log_rotate(path)                 log_create(path, log_level)

#endif /* __CODA_LOGGER_H__ */
