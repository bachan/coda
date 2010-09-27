#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "daemon.h"
#include "logger.h"
#include "system.h"

static struct option coda_getopt_opts [] =
{
	{ "config"       , 1, NULL, 'c' },
	{ "pid"	         , 1, NULL, 'p' },
	{ "daemon"       , 0, NULL, 'd' },
	{ "no-daemon"    , 0, NULL, 'D' },
	{  NULL          , 0, NULL,  0  },
};

#define CODA_GETOPT_OPTS "c:p:dD" /* don't forget to change this after changing the array above */

int coda_getopt_usage(int argc CODA_UNUSED, char** argv)
{
	msg("Usage:                                \n"
		"  %s <opts>                           \n"
		"                                      \n"
		"Options:                              \n"
		"  -c, --config <file> : custom config \n"
		"  -p, --pid    <file> : custom pid    \n"
		"  -d, --daemon        : detach        \n"
		"  -D, --no-daemon     : do not detach \n"
		"                                      "
	, argv[0]);

	return 0;
}

int coda_getopt_parse(int argc, char** argv, coda_getopt_t* opt)
{
	int c;

	coda_clrptr(opt); /* TODO coda_clrptr */

	while (-1 != (c = getopt_long(argc, argv, CODA_GETOPT_OPTS,
		coda_getopt_opts, NULL)))
	{
		switch (c)
		{
		case 'c': opt->config = optarg; break;
		case 'p': opt->pid    = optarg; break;
		case 'd': opt->daemon = 1; break;
		case 'D': opt->daemon = 0; break;
		default : return -1;
		}
	}

	return 0;
}

int coda_daemon_fork()
{
    int fd;

    switch (fork())
    {
    case -1: return -1;
    case  0: break;
    default: exit(EXIT_SUCCESS);
    }

    if (0 > (int) setsid())
    {
        return -1;
    }

    umask(0);

    if (0 > (fd = open("/dev/null", O_RDWR)))
    {
        return -1;
    }

    if (0 > dup2(fd, STDIN_FILENO))
    {
        return -1;
    }

    if (0 > dup2(fd, STDOUT_FILENO))
    {
        return -1;
    }

    if (fd > STDERR_FILENO && 0 > close(fd))
    {
        return -1;
    }

    return 0;
}

int coda_daemon_load(coda_getopt_t* opt)
{
	if (0 > coda_signal_init())
	{
		return -1;
	}

	if (opt->daemon && 0 > coda_daemon_fork())
	{
		return -1;
	}

	if (opt->pid && 0 > coda_mkpidf(opt->pid))
	{
		return -1;
	}

	return 0;
}

int coda_daemon_stop(coda_getopt_t* opt)
{
	if (opt->pid && 0 > unlink(opt->pid))
	{
		return -1;
	}

	return 0;
}

/* 
 * SIGIO is equal to SIGPOLL, so it was removed from the list, SIGCLD is
 * SIGCHLD equivalent, so it was removed too.
 */

#define CODA_SIG_LIST(_,...) \
/*  _(KILL   , evanescen, ##__VA_ARGS__) */ \
/*  _(STOP   , evanescen, ##__VA_ARGS__) */ \
/*  _(ILL    , dangerous, ##__VA_ARGS__) */ \
/*  _(ABRT   , dangerous, ##__VA_ARGS__) */ \
/*  _(FPE    , dangerous, ##__VA_ARGS__) */ \
/*  _(SEGV   , dangerous, ##__VA_ARGS__) */ \
/*  _(BUS    , dangerous, ##__VA_ARGS__) */ \
/*  _(IOT    , dangerous, ##__VA_ARGS__) */ \
    _(HUP    , changecfg, ##__VA_ARGS__)    \
    _(INT    , terminate, ##__VA_ARGS__)    \
    _(TERM   , terminate, ##__VA_ARGS__)    \
    _(USR1   , rotatelog, ##__VA_ARGS__)    \
    _(USR2   , changebin, ##__VA_ARGS__)    \
    _(ALRM   , ignoremsg, ##__VA_ARGS__)    \
    _(QUIT   , ignoremsg, ##__VA_ARGS__)    \
    _(PIPE   , ignoremsg, ##__VA_ARGS__)    \
    _(CHLD   , ignoremsg, ##__VA_ARGS__)    \
    _(CONT   , ignoremsg, ##__VA_ARGS__)    \
    _(TSTP   , ignoremsg, ##__VA_ARGS__)    \
    _(TTIN   , ignoremsg, ##__VA_ARGS__)    \
    _(TTOU   , ignoremsg, ##__VA_ARGS__)    \
    _(POLL   , ignoremsg, ##__VA_ARGS__)    \
    _(PROF   , ignoremsg, ##__VA_ARGS__)    \
    _(SYS    , ignoremsg, ##__VA_ARGS__)    \
    _(TRAP   , ignoremsg, ##__VA_ARGS__)    \
    _(URG    , ignoremsg, ##__VA_ARGS__)    \
    _(VTALRM , ignoremsg, ##__VA_ARGS__)    \
    _(XCPU   , ignoremsg, ##__VA_ARGS__)    \
    _(XFSZ   , ignoremsg, ##__VA_ARGS__)    \
    _(STKFLT , ignoremsg, ##__VA_ARGS__)    \
/*  _(IO     , ignoremsg, ##__VA_ARGS__) */ \
/*  _(CLD    , ignoremsg, ##__VA_ARGS__) */ \
    _(PWR    , ignoremsg, ##__VA_ARGS__)    \
    _(WINCH  , ignoremsg, ##__VA_ARGS__)    \
/*  _(UNUSED , ignoremsg, ##__VA_ARGS__) */ \
/*  _(EMT    , ignoremsg, ##__VA_ARGS__) */

#define CODA_SIG_CASE(name,hand) case SIG##name: log_notice(QUOTES_NAME(SIG##name)"("QUOTES_DATA(SIG##name)"): "#hand); CODA_SIG_CASE_##hand; break;
#define CODA_SIG_CASE_DEFAULT(n) default: log_notice("DEFAULT CASE in coda_signal_handle(%d) - sigaction is not defined", n); break;
#define CODA_SIG_INIT(name,hand,glob_hand) if (SIG_ERR == signal((SIG##name), (glob_hand))) return -1;
#define CODA_SIG_MASK(name,hand,sset,mask) if (((mask) & (1 << (SIG##name))) && 0 > sigaddset(&(sset),(SIG##name))) return -1;
#define CODA_SIG_CASE_terminate coda_terminate = 1
#define CODA_SIG_CASE_changecfg coda_changecfg = 1
#define CODA_SIG_CASE_rotatelog coda_rotatelog = 1
#define CODA_SIG_CASE_changebin coda_changebin = 1
#define CODA_SIG_CASE_ignoremsg /* void */

volatile int coda_terminate = 0;
volatile int coda_changecfg = 0;
volatile int coda_rotatelog = 0;
volatile int coda_changebin = 0;

static
void coda_signal_handle(int sig)
{
    switch (sig)
    {
    CODA_SIG_LIST(CODA_SIG_CASE)
	CODA_SIG_CASE_DEFAULT(sig)
    }
}

int coda_signal_init()
{
    CODA_SIG_LIST(CODA_SIG_INIT, coda_signal_handle)
    return 0;
}

int coda_signal_mask(int how, unsigned mask)
{
	sigset_t sset;
	sigemptyset(&sset);
	CODA_SIG_LIST(CODA_SIG_MASK, sset, mask);
	return sigprocmask(how, &sset, NULL); /* TODO check if this is okay with threads (nowdays) or use pthread_sigmask */
}

