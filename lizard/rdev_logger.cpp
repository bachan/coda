#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <map>
#include "rdev_stat.h"
#include "rdev_logger.h"

#define BUFSZ 4096

#define CKNN(expr)  if (0 != (expr)) return -1;

#define DATE(buf, sz, ntm) snprintf(buf, sz, "%04u/%02u/%02u %02u:%02u:%02u", ntm.tm_year + 1900, ntm.tm_mon + 1, ntm.tm_mday, ntm.tm_hour, ntm.tm_min, ntm.tm_sec)
#define LEVL(buf, sz, lev) snprintf(buf, sz, " %*.s[%s] *", 6 - log_lvs[lev].len, "", log_lvs[lev].data)
#define MESG(buf, sz, msg) snprintf(buf, sz, ": %s\n", msg)

#if (RDEV_USE_THREADS)
#include <pthread.h>
static pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
#define RDLOCK(rwlk)  pthread_rwlock_rdlock(&(rwlk))  /* rwlock-stubs */
#define WRLOCK(rwlk)  pthread_rwlock_wrlock(&(rwlk))
#define UNLOCK(rwlk)  pthread_rwlock_unlock(&(rwlk))
#define RDOP(oper)  RDLOCK(rw); int rdoprc = oper; UNLOCK(rw); if (0 != rdoprc) return -1;  /* rwlock-incapsulated operations */
#define WROP(oper)  WRLOCK(rw); int wroprc = oper; UNLOCK(rw); if (0 != wroprc) return -1;  /* rwlock-incapsulated operations */
#define LSET(id, log)  log_data (log);
#define LGET(id, log)  log_data (log); RDOP(logread((id), (log)));
#define MGET(id, log)  msg_data (msg); RDOP(msgread((id), (msg)));
#define SELF(buf, sz) snprintf(buf, sz, "%06lu", pthread_self() % 1000000)
#else
#define RDOP(oper)  /* void */
#define WROP(oper)  /* void */
#define LSET(id, log)  log_data &(log) = logs[id];
#define LGET(id, log)  log_data &(log) = logs[id];
#define MGET(id, log)  msg_data &(msg) = logs[id].msg;
#define SELF(buf, sz) 0
#endif /* (RDEV_USE_THREADS) */

struct lev_data
{
    const char *data;
    int len;
};

struct msg_data
{
    int lv;
    int fd;
};

struct log_data
{
    msg_data msg;
    char fn [BUFSZ];

public:
    log_data()
    {
        msg.lv = -1;
        msg.fd = -1;

        *fn = 0;
    }
};

typedef std::map<int, log_data> log_map;

static log_map logs;

static lev_data log_lvs [] =
{
    { "access" , sizeof("access") - 1 },  /* RDEV_LOG_ACCESS */
    { "emerg"  , sizeof("emerg" ) - 1 },  /* RDEV_LOG_EMERG  */
    { "alert"  , sizeof("alert" ) - 1 },  /* RDEV_LOG_ALERT  */
    { "crit"   , sizeof("crit"  ) - 1 },  /* RDEV_LOG_CRIT   */
    { "error"  , sizeof("error" ) - 1 },  /* RDEV_LOG_ERROR  */
    { "warn"   , sizeof("warn"  ) - 1 },  /* RDEV_LOG_WARN   */
    { "notice" , sizeof("notice") - 1 },  /* RDEV_LOG_NOTICE */
    { "info"   , sizeof("info"  ) - 1 },  /* RDEV_LOG_INFO   */
    { "debug"  , sizeof("debug" ) - 1 },  /* RDEV_LOG_DEBUG  */
    {  NULL    ,                    0 },  /* ... */
};

/* aux functions */

static int logwhat(const char *s, size_t len)
{
    for (int lv = 0; log_lvs[lv].data; ++lv)
    {
        if (0 == strncasecmp(s, log_lvs[lv].data, len))
        {
            return lv;
        }
    }

    return -1;
}

static int logexit(log_data &log)
{
    if (-1 != log.msg.fd && 0 != close(log.msg.fd))
    {
        return -1;
    }

    return 0;
}

static int logopen(log_data &log)
{
    CKNN(logexit(log));

    if (RDEV_LOG_ACCESS > log.msg.lv || RDEV_LOG_DEBUG < log.msg.lv)
    {
        return -1;
    }

    CKNN(rdev_pmkdir(log.fn, 0755));

    if (-1 == (log.msg.fd = open(log.fn, O_CREAT|O_APPEND|O_WRONLY, 0644)))
    {
        return -1;
    }

    return 0;
}

#if (RDEV_USE_THREADS)
static int logread(int id, log_data &log)
{
    log_map::const_iterator it = logs.find(id);
    if (logs.end() == it) return -1;

    const log_data &src = it->second;

    log.msg.lv = src.msg.lv;
    log.msg.fd = src.msg.fd;
    snprintf(log.fn, BUFSZ, "%s", src.fn);

    return 0;
}

static int msgread(int id, msg_data &msg)
{
    log_map::const_iterator it = logs.find(id);
    if (logs.end() == it) return -1;

    const log_data &src = it->second;

    msg.lv = src.msg.lv;
    msg.fd = src.msg.fd;

    return 0;
}

static int logpass(int id, const log_data &log)
{
    log_data &dst = logs[id];

    dst.msg.lv = log.msg.lv;
    dst.msg.fd = log.msg.fd;
    snprintf(dst.fn, BUFSZ, "%s", log.fn);

    return 0;
}
#endif /* (RDEV_USE_THREADS) */

/* main */

int rdev_log_snum(int id, const char *fn, int lv)
{
    LSET(id, log);

    snprintf(log.fn, BUFSZ, "%s", fn);
    log.msg.lv = lv;

    CKNN(logopen(log));
    WROP(logpass(id, log));

    return 0;
}

int rdev_log_sstr(int id, const char *fn, const char *ls)
{
    return rdev_log_snum(id, fn, logwhat(ls, strlen(ls)));
}

int rdev_log_sacc(int id, const char *fn)
{
    return rdev_log_snum(id, fn, RDEV_LOG_ACCESS);
}

int rdev_log_snum_fd(int id, int fd, int lv)
{
    LSET(id, log);

    snprintf(log.fn, BUFSZ, "%s", "/");
    log.msg.lv = lv;
    log.msg.fd = fd;

    if (RDEV_LOG_ACCESS > log.msg.lv || RDEV_LOG_DEBUG < log.msg.lv)
    {
        return -1;
    }
    WROP(logpass(id, log));

    return 0;

}

int rdev_log_sstr_fd(int id, int fd, const char *ls)
{
    return rdev_log_snum_fd(id, fd, logwhat(ls, strlen(ls)));
}

int rdev_log_sacc_fd(int id, int fd)
{
    return rdev_log_snum_fd(id, fd, RDEV_LOG_ACCESS);
}

int rdev_log_get_lv (int id)
{
    MGET(id, msg);
    if (-1 == msg.fd) return -1;
    return msg.lv;
}

int rdev_log_rotate(int id)
{
    LGET(id, log);
    CKNN(logopen(log));
    WROP(logpass(id, log));

    return 0;
}

int rdev_log_close(int id)
{
    LGET(id, log);
    CKNN(logexit(log));
    WROP(logs.erase(id));

    return 0;
}

int rdev_log_message(int id, int lv, const char *fmt, ...)
{
    MGET(id, msg);

    if (-1 == msg.fd) return -1;
    if (lv  > msg.lv) return -1;

    char sb [BUFSZ];
    char wb [BUFSZ];

    va_list ap; va_start(ap, fmt);
    vsnprintf(sb, BUFSZ, fmt, ap);

    time_t ts; struct tm nt;
    time(&ts); localtime_r(&ts, &nt);

    /* log */

    int bc = 0;

    bc += DATE(wb + bc, BUFSZ - bc, nt);
    bc += LEVL(wb + bc, BUFSZ - bc, lv);
    bc += SELF(wb + bc, BUFSZ - bc);
    bc += MESG(wb + bc, BUFSZ - bc, sb);

    /* write */

    return (0 <= write(msg.fd, wb, bc)) ? 0 : -1;
}

