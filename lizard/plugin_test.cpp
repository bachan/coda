#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "rdev_logger.h"
#include "rdev_error.hpp"
#include "plugin.hpp"

class lz_test : public lizard::plugin
{
    lizard::server_callback *srv;

public:
     lz_test(lizard::server_callback *srv_cb);
    ~lz_test();

    int set_param(const char *xml_in);
    void idle();

    int handle_easy(lizard::task *task);
    int handle_hard(lizard::task *task);
};

extern "C" lizard::plugin *get_plugin_instance(lizard::server_callback *srv_cb)
{
    srv_cb->log_message(lizard::log_info, "HELLO "
        "from get_plugin_instance");
    
    try
    {
        return new lz_test (srv_cb);
    }
    catch (const std::exception &e)        
    {
        srv_cb->log_message(lizard::log_crit, "lz_test plugin "
            "load failed: %s", e.what());
    }

    return NULL;
}

lz_test:: lz_test(lizard::server_callback *srv_cb) : plugin(srv_cb), srv(srv_cb)
{
}

lz_test::~lz_test()
{
}

int lz_test::set_param(const char *xml_in)
{
    srv->log_message(lizard::log_info, "loading "
        "plugin param: %s", xml_in);

    return rSuccess;
}

int lz_test::handle_easy(lizard::task *task)
{
    switch (task->get_request_method())
    {
        case lizard::task::requestGET:   break;
        case lizard::task::requestHEAD:  break;
        case lizard::task::requestPOST:  return rHard;
        case lizard::task::requestUNDEF: return rError;
    }

    task->set_response_status  (200);
    task->append_response_body ("HAI\n", 4);

    return rSuccess;
}

int lz_test::handle_hard(lizard::task* /*task*/)
{
    return rSuccess;
}

void lz_test::idle()
{
}

