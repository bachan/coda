#ifndef __LOGGER_H__
#define __LOGGER_H__

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

#define log_format(fd,lv,str,err,fmt,...) do {      \
                                                    \
    if (lv <= log_lvcurr)                           \
    {                                               \
        dprintf(fd, str " *: " fmt " (%d: %s)",     \
            ##__VA_ARGS__, err, rdv_errstr(err));   \
    }                                               \
                                                    \
} while (0)

extern volatile
int log_lvcurr;
int log_lvread(const char* lv);
int log_create(const char* fn, int lv);

#define log_string(fn,lv) log_create(fn, log_lvread(lv))
#define log_rotate(fn)    log_create(fn, log_lvcurr)

#endif /* __LOGGER_H__ */
