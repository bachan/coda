#ifndef __CODA_LOGGER_H__
#define __CODA_LOGGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_ACCESS  0  /* ACC: special level for access log      */
#define LOG_EMERG   1  /* EME: system is unusable                */
#define LOG_ALERT   2  /* ALE: action must be taken immediately  */
#define LOG_CRIT    3  /* CRI: critical conditions               */
#define LOG_ERROR   4  /* ERR: error conditions                  */
#define LOG_WARN    5  /* WRN: warning conditions                */
#define LOG_NOTICE  6  /* NOT: normal, but significant condition */
#define LOG_INFO    7  /* NFO: informational message             */

#define log_access(e,fmt,...) log_format(STDERR_FILENO,LOG_ACCESS,"[access]",e,fmt,##__VA_ARGS__)
#define log_emerg( e,fmt,...) log_format(STDERR_FILENO,LOG_EMERG, " [emerg]",e,fmt,##__VA_ARGS__)
#define log_alert( e,fmt,...) log_format(STDERR_FILENO,LOG_ALERT, " [alert]",e,fmt,##__VA_ARGS__)
#define log_crit(  e,fmt,...) log_format(STDERR_FILENO,LOG_CRIT,  "  [crit]",e,fmt,##__VA_ARGS__)
#define log_error( e,fmt,...) log_format(STDERR_FILENO,LOG_ERROR, " [error]",e,fmt,##__VA_ARGS__)
#define log_warn(  e,fmt,...) log_format(STDERR_FILENO,LOG_WARN,  "  [warn]",e,fmt,##__VA_ARGS__)
#define log_notice(e,fmt,...) log_format(STDERR_FILENO,LOG_NOTICE,"[notice]",e,fmt,##__VA_ARGS__)
#define log_info(  e,fmt,...) log_format(STDERR_FILENO,LOG_INFO,  "  [info]",e,fmt,##__VA_ARGS__)

#define log_format(fd,lv,ls,fmt,...) do {                               \
                                                                        \
    if (lv <= log_lvmask)                                               \
    {                                                                   \
        struct tm tp;                                                   \
                                                                        \
        rdv_gmtime(time(NULL), &tp);                                    \
                                                                        \
        dprintf(fd, "%04u-%02u-%02u-%02u:%02u:%02u\t"ls"\t%s:\t"fmt"\n",\
            tp.tm_year,                                                 \
            tp.tm_mon,                                                  \
            tp.tm_mday,                                                 \
            tp.tm_hour,                                                 \
            tp.tm_min,                                                  \
            tp.tm_sec,                                                  \
            log_get_thread_name(),                                      \
            ##__VA_ARGS__                                               \
        );                                                              \
    }                                                                   \
                                                                        \
} while (0)

extern volatile int log_lvmask;

int log_lvread(const char* lv);
int log_create_int(const char* fn, int lv);

int log_set_thread_name(const char* name);
const char* log_get_thread_name();

#define log_set_thread_name(name) pthread_setspecific(log_thread_name, (const void*) name)
#define log_get_thread_name() (const char*) pthread_getspecific(log_thread_name)

#define log_create_str(fn,lv) log_create_int(fn, log_lvread(lv))
#define log_rotate(fn) log_create_int(fn, log_lvmask)

#ifdef __cplusplus
}
#endif

#endif /* __CODA_LOGGER_H__ */

