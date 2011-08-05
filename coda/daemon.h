#ifndef __CODA_DAEMON_H__
#define __CODA_DAEMON_H__

#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct coda_getopt
	coda_getopt_t;

struct coda_getopt
{
	const char* config;
	const char* pid;

	unsigned daemon:1;
};

int coda_getopt_usage(int argc, char** argv);
int coda_getopt_parse(int argc, char** argv, coda_getopt_t* opt);

int coda_daemon_fork();
int coda_daemon_load(coda_getopt_t* opt);
int coda_daemon_stop(coda_getopt_t* opt);

extern volatile int coda_terminate;
extern volatile int coda_changecfg;
extern volatile int coda_rotatelog;
extern volatile int coda_changebin;

int coda_signal_init();
int coda_signal_init_norestart();
int coda_signal_mask(int how, unsigned mask);

#ifdef __cplusplus
}
#endif

#endif /* __CODA_DAEMON_H__ */
