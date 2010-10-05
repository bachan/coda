#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <exception>
#include <blizzard/plugin.hpp>
#include <coda/logger.h>
#include <coda/txml.hpp>

struct blzmod_example : public blz_plugin
{
	 blzmod_example() {}
	virtual ~blzmod_example() throw() {}

	int load(const char* cfg);
	int idle();

	int easy(blz_task* task);
	int hard(blz_task* task);
};

extern "C" blz_plugin* get_plugin_instance()
{
	try
	{
	    return new blzmod_example ();
	}
	catch (const std::exception& e)        
	{
	    log_crit("load blzmod_example plugin failed: %s", e.what());
	}

	return NULL;
}

int blzmod_example::load(const char* cfg)
{
	log_notice("loading plugin: %s", cfg);
	return BLZ_OK;
}

int blzmod_example::easy(blz_task* task)
{
	switch (task->get_request_method())
	{
		case BLZ_METHOD_GET:   break;
		case BLZ_METHOD_HEAD:  break;
		case BLZ_METHOD_POST:  return BLZ_AGAIN;
		case BLZ_METHOD_UNDEF: return BLZ_ERROR;
	}

	task->set_response_status(200);
	task->add_response_header("X-Generated-By", "blzmod_example");
	task->add_response_buffer("BODY\n", sizeof("BODY\n") - 1);

	return BLZ_OK;
}

int blzmod_example::hard(blz_task*)
{
	return BLZ_OK;
}

int blzmod_example::idle()
{
	return BLZ_OK;
}

