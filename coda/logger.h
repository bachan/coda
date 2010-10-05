#ifndef __CODA_LOGGER_H__
#define __CODA_LOGGER_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include "gmtime.h"
#include "system.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_access  0  /* ACC: special level for access log      */
#define LOG_emerg   1  /* EME: system is unusable                */
#define LOG_alert   2  /* ALE: action must be taken immediately  */
#define LOG_crit    3  /* CRI: critical conditions               */
#define LOG_error   4  /* ERR: error conditions                  */
#define LOG_warn    5  /* WRN: warning conditions                */
#define LOG_notice  6  /* NOT: normal, but significant condition */
#define LOG_info    7  /* NFO: informational message             */
#define LOG_debug   8  /* DBG: debug message                     */

#define log_msg(lev,fmt,...) log_fmt(STDERR_FILENO,LOG_##lev,#lev,fmt,##__VA_ARGS__)
#define log_err(err,fmt,...) log_msg(error,fmt " (%d: %s)\n", ##__VA_ARGS__, err, coda_strerror(err))
#define log_ret(err,fmt,...) log_msg(error,fmt " (%d: %s)\n", ##__VA_ARGS__, err, coda_strerror(err)), err
#define log_die(err,fmt,...) log_msg(emerg,fmt " (%d: %s)\n", ##__VA_ARGS__, err, coda_strerror(err)); exit(EXIT_FAILURE)

#define msg(fmt,...)     dprintf(STDERR_FILENO, fmt          "\n", ##__VA_ARGS__)
#define err(err,fmt,...) dprintf(STDERR_FILENO, fmt " (%d: %s)\n", ##__VA_ARGS__, err, coda_strerror(err))
#define ret(err,fmt,...) dprintf(STDERR_FILENO, fmt " (%d: %s)\n", ##__VA_ARGS__, err, coda_strerror(err)), err
#define die(err,fmt,...) dprintf(STDERR_FILENO, fmt " (%d: %s)\n", ##__VA_ARGS__, err, coda_strerror(err)); exit(EXIT_FAILURE)

#define log_access( fmt,...) log_msg(access,fmt,##__VA_ARGS__)
#define log_emerg(  fmt,...) log_msg(emerg, fmt,##__VA_ARGS__)
#define log_alert(  fmt,...) log_msg(alert, fmt,##__VA_ARGS__)
#define log_crit(   fmt,...) log_msg(crit,  fmt,##__VA_ARGS__)
#define log_error(  fmt,...) log_msg(error, fmt,##__VA_ARGS__)
#define log_warn(   fmt,...) log_msg(warn,  fmt,##__VA_ARGS__)
#define log_notice( fmt,...) log_msg(notice,fmt,##__VA_ARGS__)
#define log_info(   fmt,...) log_msg(info,  fmt,##__VA_ARGS__)
#ifndef NDEBUG
#define log_debug(  fmt,...) log_msg(debug, fmt,##__VA_ARGS__)
#else
#define log_debug(  fmt,...) /* nothing here */
#endif /* NDEBUG */

#define LOG_FORMAT(lvstr,tname,fmt) "%04u-%02u-%02u %02u:%02u:%02u\t"lvstr"\t"tname"\t"fmt"\n"
#define LOG_VALUES(tmgmt,tname,...) tmgmt.tm_year, tmgmt.tm_mon, tmgmt.tm_mday, \
    tmgmt.tm_hour, tmgmt.tm_min, tmgmt.tm_sec, tname, ##__VA_ARGS__

#define log_fmt(fd,level,lvstr,fmt,...) do {								\
                                                                            \
    if (level <= log_level)                                                 \
    {                                                                       \
        struct tm tmgmt;                                                    \
        const char* tname;                                                  \
                                                                            \
        coda_gmtime(time(NULL), &tmgmt);                                    \
                                                                            \
        if (NULL != (tname = log_thread_name_get()))                        \
        {                                                                   \
            dprintf(fd, LOG_FORMAT(lvstr,"%s",fmt),                         \
                        LOG_VALUES(tmgmt,tname,##__VA_ARGS__));             \
        }                                                                   \
        else                                                                \
        {                                                                   \
            dprintf(fd, LOG_FORMAT(lvstr,"%lu",fmt),                        \
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

#ifdef __cplusplus
}
#endif

#endif /* __CODA_LOGGER_H__ */
