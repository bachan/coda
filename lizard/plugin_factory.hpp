#ifndef T_SERV_PLUGIN_FACTORY_HPP___
#define T_SERV_PLUGIN_FACTORY_HPP___

#include <pthread.h>
#include "config.hpp"
#include "plugin.hpp"

namespace lizard
{

class plugin_factory
{
    void* loaded_module;
    lizard::plugin* plugin_handle;

public:
    plugin_factory();
    ~plugin_factory();

    void load_module(const lizard::lz_config::ROOT::PLUGIN& pd);
    void unload_module();

    lizard::plugin* get_plugin() const;

    void idle();
};

}

#endif
