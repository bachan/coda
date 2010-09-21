#include <getopt.h>
#include <signal.h>
#include <sys/time.h>
#include <coda/error.h>
#include <coda/logger.h>
#include "server.hpp"

namespace lizard {

volatile sig_atomic_t quit   = 0;
volatile sig_atomic_t hup    = 0;
volatile sig_atomic_t rotate = 0;

int MSG_LIZARD_ID = 5467;

} /* namespace lizard */

void blink()
{
    struct timespec ts = { 0, 500000 };
    nanosleep(&ts, 0);
}

static void onPipe(int /*v*/)
{
}

static void onTerm(int /*v*/)
{
    lizard::quit = 1;
    log_warn("TERM received");
}

static void onInt(int /*v*/)
{
    lizard::quit = 1;
    log_warn("INT received");
}

static void onHUP(int /*v*/)
{
    lizard::hup = 1;
}

static void onUSR1(int /*v*/)
{
    lizard::rotate = 1;
}

int main(int argc, char * argv[])
{
    if (2 > argc)
    {
        fprintf(stderr, "Usage: %s [-D,--no-daemon] [-p,--pid-file=] "
            "[-c,--config-file=] [-v,--version]\n", argv[0]);

        exit(EXIT_FAILURE);
    }

    //-----------------------------------------------
    //begin parsing incoming parameters

    std::string pid_file_name;
    std::string cfg_file_name;
    bool no_daemon = false;

    static struct option long_options[] = 
    {
        {"no-daemon",   0,  0, 'D'},
        {"pid-file",    1,  0, 'p'},
        {"config-file", 1,  0, 'c'},
        {"version",     0,  0, 'v'},
        {0,             0,  0,  0 }
    };
    int option_index = 0;
    int c;

    while(-1 != (c = getopt_long(argc, argv, "vDc:p:", long_options, &option_index)))
    {
        switch(c)
        {
            case 'v':
                printf("lizard/%s\n", LIZARD_STR_VERSION); 
                break;

            case 'c':
                cfg_file_name = optarg;
                break;

            case 'p':
                pid_file_name = optarg;
                break;

            case 'D':
                no_daemon = true;
                break;

            default:
                break;
        }
    }

    //-----------------------------------------------
    //dealing with params extracted

    if(cfg_file_name.empty())
    {
        fprintf(stderr, "--config-file is not set\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGTERM, onTerm);
    signal(SIGINT,  onInt);
    signal(SIGHUP,  onHUP);
    signal(SIGPIPE, onPipe);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGUSR1, onUSR1);
    signal(SIGUSR2, SIG_IGN);

    try
    {
        lizard::server server;

        if(false == no_daemon)
        {
            int res = fork();
            if(-1 == res)
            {
                char buff[100];
                fprintf(stderr, "fork() error: %s\n", strerror_r(errno, buff, 100));

                exit(3);
            }

            if(res)
            {
                exit(0);
            }

            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);

            if(-1 == setsid())
            {
                exit(EXIT_FAILURE);
            }
        }

        while(!lizard::quit)
        {
            log_info("loading config...");
            server.load_config(cfg_file_name.c_str(), pid_file_name);

            log_info("prepare...");
            server.prepare();

            log_info("init lizard...");

            if(!pid_file_name.empty() && false == lz_utils::pid_file_init(pid_file_name.c_str()))
            {
				log_die(errno, "create pid-file failed");
            }

            server.init_threads();
            log_info("all threads started!");

            server.join_threads();
            log_info("all threads ended!");

            if(!pid_file_name.empty())
            {
                if(false == lz_utils::pid_file_free(pid_file_name.c_str()))
                {
					log_die(errno, "free pid-file failed");
                }
            }

            log_info("finalizing...");
            server.finalize();

            lizard::hup = 0;
        }
    }
    catch (const std::exception &e)
    {
        lizard::quit = 1;

        if (no_daemon) fprintf(stderr, "main: exception: %s\n", e.what());
        log_crit("main: exception: %s", e.what());

        if (!pid_file_name.empty())
        {
            if (false == lz_utils::pid_file_free(pid_file_name.c_str()))
            {
				log_die(errno, "free pid-file failed");
            }
        }
    }

    return 0;
}

