#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include "logger.h"

volatile int log_lvmask = LOG_INFO;

pthread_key_t logger = ...;

pthread_setspecific(logger, name);



int log_lvread(const char* lv)
{
    switch (lv[0])
    {
    case 'a': case 'A':
        switch (lv[1])
        {
        case 'c': case 'C': return LOG_ACCESS;
        case 'l': case 'L': return LOG_ALERT;
        }
        break;
    case 'c': case 'C': return LOG_CRIT;
    case 'e': case 'E':
        switch (lv[1])
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

int log_create(const char* fn, int lv)
{
    if (0 > mkpath((char *) fn)) return -1;
    if (0 > fdopen(STDERR_FILENO, fn, O_CREAT|O_APPEND|O_WRONLY))
        return -1;

    log_lvmask = lv;

    return STDERR_FILENO;
}


pthread_key_t log_thread_name;

int log_set_thread_name(const char* name)
{
    pthread_setspecific(log_thread_name, (const void*) name);
}


