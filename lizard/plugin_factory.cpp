#include <dlfcn.h>
#include "server.hpp"

lizard::plugin_factory::plugin_factory() : loaded_module(0), plugin_handle(0)
{

}

lizard::plugin_factory::~plugin_factory()
{
    unload_module();
}

void lizard::plugin_factory::load_module(const lizard::lz_config::ROOT::PLUGIN& pd)
{
    if(0 == pd.easy_threads)
    {
        return;
    }

    if(0 == loaded_module)
    {
        loaded_module = dlopen(pd.library.c_str(), RTLD_NOW | RTLD_LOCAL);
        if(loaded_module == 0)
        {
            char buff[2048];
            snprintf(buff, 2048, "loading module '%s' failed: %s", pd.library.c_str(), dlerror());

            throw std::logic_error(buff);
        }
    }
    else
    {
        throw std::logic_error("module already loaded!!!");
    }

    dlerror();

    //------------------ UGLY evil hack -----------------
    union conv_union
    {
        void* v;
        lizard::plugin* (*f)();
    } conv;

    conv.v = dlsym(loaded_module, "get_plugin_instance");
    lizard::plugin* (*func)() = conv.f;
    //---------------------------------------------------

    const char * errmsg = dlerror();
    if(0 != errmsg)
    {
        char buff[2048];
        snprintf(buff, 2048, "error searching 'get_plugin_instance' in module '%s': '%s'", pd.library.c_str(), errmsg);

        throw std::logic_error(buff);
    }

    plugin_handle = (*func)();

    if(0 == plugin_handle)
    {
        char buff[2048];
        snprintf(buff, 2048, "module '%s': instance of plugin is not created", pd.library.c_str());

        throw std::logic_error(buff);
    }

    if(plugin::rSuccess != plugin_handle->set_param(pd.params.c_str()))
    {
        throw std::logic_error("Plugin init failed");
    }

}

void lizard::plugin_factory::unload_module()
{
    if(plugin_handle)
    {
        delete plugin_handle;
        plugin_handle = 0;
    }

    if(loaded_module)
    {
        dlclose(loaded_module);
        loaded_module = 0;
    }
}

lizard::plugin * lizard::plugin_factory::get_plugin()const
{
    return plugin_handle;
}

void lizard::plugin_factory::idle()
{
    if(plugin_handle)
    {
        plugin_handle->idle();
    }
}
