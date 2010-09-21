#ifndef __T_SERV_CONFIG___
#define __T_SERV_CONFIG___

#include <string>
#include <inttypes.h>
#include <coda/error.hpp>
#include <coda/logger.h>
#include <coda/txml.hpp>

#define SRV_BUF 4096

namespace lizard {

struct lz_config : public coda::txml_determination_object
{
    struct ROOT : public coda::txml_determination_object
    {
        std::string pid_file_name;
        std::string log_file_name;
        std::string log_level;

        std::string access_log_file_name;

        struct STATS : public coda::txml_determination_object
        {
            std::string ip;
            std::string port;

            STATS(){}

            void determine(coda::txml_parser* p)
            {
                p->determineMember("ip", ip);
                p->determineMember("port", port);
            }

            void clear()
            {
                ip.clear();
                port.clear();
            }

            void check(const char *par, const char *ns)
            {
                char curns [SRV_BUF];
                snprintf(curns, SRV_BUF, "%s:%s", par, ns);

                if (ip  .empty()) throw coda_error ("<%s:ip> is empty in config", curns);
                if (port.empty()) throw coda_error ("<%s:port> is empty in config", curns);
            }
        };

        struct PLUGIN : public coda::txml_determination_object
        {
            std::string ip;
            std::string port;
            int connection_timeout;
            int idle_timeout;

            std::string library;
            std::string params;

            int easy_threads;
            int hard_threads;

            int easy_queue_limit;
            int hard_queue_limit;

            PLUGIN() : connection_timeout(0), idle_timeout(0), easy_threads(1), hard_threads(0), easy_queue_limit(0), hard_queue_limit(0){}

            void determine(coda::txml_parser* p)
            {
                p->determineMember("ip", ip);
                p->determineMember("port", port);
                p->determineMember("connection_timeout", connection_timeout);
                p->determineMember("idle_timeout", idle_timeout);

                p->determineMember("library", library);
                p->determineMember("params", params);

                p->determineMember("easy_threads", easy_threads);
                p->determineMember("hard_threads", hard_threads);

                p->determineMember("easy_queue_limit", easy_queue_limit);
                p->determineMember("hard_queue_limit", hard_queue_limit);
            }

            void clear()
            {
                ip.clear();
                port.clear();
                connection_timeout = 0;
                idle_timeout = 0;

                library.clear();
                params.clear();

                easy_threads = 1;
                hard_threads = 0;

                easy_queue_limit = 0;
                hard_queue_limit = 0;
            }

            void check(const char *par, const char *ns)
            {
                char curns [SRV_BUF];
                snprintf(curns, SRV_BUF, "%s:%s", par, ns);

                if (ip     .empty()) throw coda_error ("<%s:ip> is empty in config", curns);
                if (port   .empty()) throw coda_error ("<%s:port> is empty in config", curns);
                if (library.empty()) throw coda_error ("<%s:library> is empty in config", curns);

                if (0 == connection_timeout) throw coda_error ("<%s:connection_timeout> is not set or set to 0", curns);
                if (0 == easy_threads) throw coda_error ("<%s:easy_threads> is set to 0", curns);
            }
        };

        STATS stats;
        PLUGIN plugin;

        void determine(coda::txml_parser* p)
        {
            p->determineMember("pid_file_name", pid_file_name);
            p->determineMember("log_file_name", log_file_name);
            p->determineMember("log_level", log_level);

            p->determineMember("access_log_file_name", access_log_file_name);

            p->determineMember("stats", stats);
            p->determineMember("plugin", plugin);

        }

        void clear()
        {
            pid_file_name.clear();
            log_file_name.clear();
            log_level.clear();

            access_log_file_name.clear();

            stats.clear();
            plugin.clear();
        }

        void check()
        {
            const char *curns = "lizard";

            if (log_level.empty()) throw coda_error ("<%s:log_level> is empty in config", curns);

            stats .check(curns, "stats");
            plugin.check(curns, "plugin");
        }
    } root;

    void determine(coda::txml_parser* p)
    {
        p->determineMember("lizard", root);
    }

    void clear()
    {
        root.clear();
    }

    void check()
    {
        root.check();
    }
};

//----------------------------------------------------------------------------------
}

#endif
