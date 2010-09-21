#ifndef __RDEV_LOGGER_H__
#define __RDEV_LOGGER_H__

#include "rdev.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RDEV_LOG_ACCESS  0  /* special level for access log      */
#define RDEV_LOG_EMERG   1  /* system is unusable                */
#define RDEV_LOG_ALERT   2  /* action must be taken immediately  */
#define RDEV_LOG_CRIT    3  /* critical conditions               */
#define RDEV_LOG_ERROR   4  /* error conditions                  */
#define RDEV_LOG_WARN    5  /* warning conditions                */
#define RDEV_LOG_NOTICE  6  /* normal, but significant condition */
#define RDEV_LOG_INFO    7  /* informational message             */
#define RDEV_LOG_DEBUG   8  /* debug-level message               */

#define rdev_log_access(id, fmt ...)  rdev_log_message ((id), RDEV_LOG_ACCESS, ##fmt)
#define  rdev_log_emerg(id, fmt ...)  rdev_log_message ((id), RDEV_LOG_EMERG,  ##fmt)
#define  rdev_log_alert(id, fmt ...)  rdev_log_message ((id), RDEV_LOG_ALERT,  ##fmt)
#define   rdev_log_crit(id, fmt ...)  rdev_log_message ((id), RDEV_LOG_CRIT,   ##fmt)
#define  rdev_log_error(id, fmt ...)  rdev_log_message ((id), RDEV_LOG_ERROR,  ##fmt)
#define   rdev_log_warn(id, fmt ...)  rdev_log_message ((id), RDEV_LOG_WARN,   ##fmt)
#define rdev_log_notice(id, fmt ...)  rdev_log_message ((id), RDEV_LOG_NOTICE, ##fmt)
#define   rdev_log_info(id, fmt ...)  rdev_log_message ((id), RDEV_LOG_INFO,   ##fmt)

#ifndef  rdev_log_debug
#define  rdev_log_debug(id, fmt ...)  rdev_log_message ((id), RDEV_LOG_DEBUG,  ##fmt)
#endif

/* Устанавливает логу с [id] вывод в файл с именем [fn] с уровнем вывода [lv] (численным) */
int rdev_log_snum    (int id, const char *fn, int lv);
/* Устанавливает логу с [id] вывод в файл с именем [fn] с уровнем вывода [lv] (текстовым) */
int rdev_log_sstr    (int id, const char *fn, const char *ls);
/* Устанавливает логу с [id] вывод в файл с именем [fn] с уровнем вывода RDEV_LOG_ACCESS */
int rdev_log_sacc    (int id, const char *fn);

/* Устанавливает логу с [id] вывод в файл с дескриптором [fd] с уровнем вывода [lv] (численным) */
int rdev_log_snum_fd (int id, int fd, int lv);
/* Устанавливает логу с [id] вывод в файл с дескриптором [fd] с уровнем вывода [lv] (текстовым) */
int rdev_log_sstr_fd (int id, int fd, const char *ls);
/* Устанавливает логу с [id] вывод в файл с дескриптором [fd] с уровнем вывода RDEV_LOG_ACCESS */
int rdev_log_sacc_fd (int id, int fd);

/* Возвращает уровень вывода лога [id] или -1, если нет лога с таким id */
int rdev_log_get_lv (int id);

int rdev_log_rotate  (int id);
int rdev_log_close   (int id);

int rdev_log_message (int id, int lv, const char *fmt, ...)
    RDEV_FORMAT(printf, 3, 4);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __RDEV_LOGGER_H__ */
