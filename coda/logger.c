#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include "logger.h"

volatile int log_level = LOG_INFO;

int log_levels(const char* level)
{
    switch (level[0])
    {
    case 'a': case 'A':
        switch (level[1])
        {
        case 'c': case 'C': return LOG_ACCESS;
        case 'l': case 'L': return LOG_ALERT;
        }
        break;
    case 'c': case 'C': return LOG_CRIT;
    case 'e': case 'E':
        switch (level[1])
        {
        case 'm': case 'M': return LOG_EMERG;
        case 'r': case 'R': return LOG_ERROR;
        }
        break;
    case 'i': case 'I': return LOG_INFO;
    case 'n': case 'N': return LOG_NOTICE;
    case 'w': case 'W': return LOG_WARN;
    }

    return LOG_INFO; /* default */
}

int log_create(const char* path, int level)
{
    if (0 > coda_mkpath((char *) path))
    {
        return -1;
    }

    if (0 > coda_fdopen(STDERR_FILENO, path,
        O_CREAT|O_APPEND|O_WRONLY))
    {
        return -1;
    }

    log_level = level;

    return STDERR_FILENO;
}

/* thread */

pthread_key_t  log_thread_name;
pthread_once_t log_thread_name_once = PTHREAD_ONCE_INIT;

static
void log_thread_name_key()
{
    pthread_key_create(&log_thread_name, NULL);
}

int log_thread_name_set(const char* name)
{
    pthread_once(&log_thread_name_once,
        log_thread_name_key);

    return pthread_setspecific(log_thread_name,
        (const void*) name);
}

const char* log_thread_name_get()
{
    pthread_once(&log_thread_name_once,
        log_thread_name_key);

    return pthread_getspecific(log_thread_name);
}

