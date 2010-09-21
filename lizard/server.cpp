#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdexcept>
#include "server.hpp"

#define MSG_LIZARD_ACCESS_LOG_ID 6548

lizard::statistics stats;

//-----------------------------------------------------------------------------------------------------------
inline std::string events2string(int events)
{
    std::string rep;

    if(events & EPOLLIN)
    {
        if(!rep.empty())
            rep += " | EPOLLIN";
        else
            rep += " EPOLLIN";
    }

    if(events & EPOLLOUT)
    {
        if(!rep.empty())
            rep += " | EPOLLOUT";
        else
            rep += " EPOLLOUT";
    }

    /*if(events & EPOLLRDHUP)
    {
        if(!rep.empty())
            rep += " | EPOLLRDHUP";
        else
            rep += " EPOLLRDHUP";
    }*/

    if(events & EPOLLPRI)
    {
        if(!rep.empty())
            rep += " | EPOLLPRI";
        else
            rep += " EPOLLPRI";
    }

    if(events & EPOLLERR)
    {
        if(!rep.empty())
            rep += " | EPOLLERR";
        else
            rep += " EPOLLERR";
    }

    if(events & EPOLLHUP)
    {
        if(!rep.empty())
            rep += " | EPOLLHUP";
        else
            rep += " EPOLLHUP";
    }

    if(events & EPOLLET)
    {
        if(!rep.empty())
            rep += " | EPOLLET";
        else
            rep += " EPOLLET";
    }

    if(events & EPOLLONESHOT)
    {
        if(!rep.empty())
            rep += " | EPOLLONESHOT";
        else
            rep += " EPOLLONESHOT";
    }

    return  "[" + rep + " ]";
}

inline std::string events2string(const epoll_event& ev)
{
    std::string rep = "[";

    char buff[256];
    snprintf(buff, 256, "%d", ev.data.fd);
    rep += buff;

    rep += "]: " + events2string(ev.events);

    return rep;
}

//-----------------------------------------------------------------------------------------------------------

namespace lizard
{
    extern volatile sig_atomic_t    quit;
    extern volatile sig_atomic_t    hup;
    extern volatile sig_atomic_t    rotate;

    extern int             MSG_LIZARD_ID;

    //statistics stats;

    void * epoll_loop_function(void * ptr);
    void * easy_loop_function(void * ptr);
    void * hard_loop_function(void * ptr);
    void * stats_loop_function(void * ptr);
    void * idle_loop_function(void * ptr);
}

//-----------------------------------------------------------------------------------------------------------

lizard::server::lz_callback::lz_callback() : lz(0)
{

}

lizard::server::lz_callback::~lz_callback()
{
    lz = 0;
}


void lizard::server::lz_callback::init(server * srv)
{
    lz = srv;
}

void lizard::server::lz_callback::log_message(plugin_log_levels log_level, const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char lstr [4097];
    vsnprintf(lstr, 4096, fmt, ap);
    rdev_log_message(MSG_LIZARD_ID, log_level, "PLG: %s", lstr);
}

void lizard::server::lz_callback::vlog_message(plugin_log_levels log_level, const char * fmt, va_list ap)
{
    char lstr [4097];
    vsnprintf(lstr, 4096, fmt, ap);
    rdev_log_message(MSG_LIZARD_ID, log_level, "PLG: %s", lstr);
}

//-----------------------------------------------------------------------------------------------------------

lizard::server::server() :    incoming_sock(-1),
                       stats_sock(-1),
                       epoll_sock(-1),
                epoll_wakeup_isock(-1),
                epoll_wakeup_osock(-1),
                threads_num(0),
                start_time(0)
{
    pthread_mutex_init(&done_mutex, 0);

    pthread_mutex_init(&easy_proc_mutex, 0);
    pthread_mutex_init(&hard_proc_mutex, 0);
    pthread_mutex_init(&stats_proc_mutex, 0);

    pthread_cond_init(&easy_proc_cond, 0);
    pthread_cond_init(&hard_proc_cond, 0);
    pthread_cond_init(&stats_proc_cond, 0);

    start_time = time(0);
}

lizard::server::~server()
{
    rdev_log_debug(MSG_LIZARD_ID, "~server()");

    quit = 1;
    join_threads();

    finalize();

    fire_all_threads();

    pthread_cond_destroy(&stats_proc_cond);
    pthread_cond_destroy(&hard_proc_cond);
    pthread_cond_destroy(&easy_proc_cond);

    pthread_mutex_destroy(&stats_proc_mutex);
    pthread_mutex_destroy(&hard_proc_mutex);
    pthread_mutex_destroy(&easy_proc_mutex);

    pthread_mutex_destroy(&done_mutex);

    rdev_log_debug(MSG_LIZARD_ID, "/~server()");
}

//-----------------------------------------------------------------------------------------------------------

void lizard::server::init_threads()
{
    if(0 == pthread_create(&epoll_th, NULL, &epoll_loop_function, this))
    {
        rdev_log_debug(MSG_LIZARD_ID, "epoll thread created");
        threads_num++;
    }
    else
    {
        throw std::logic_error("error creating epoll thread");
    }

    if(0 == pthread_create(&idle_th, NULL, &idle_loop_function, this))
    {
        threads_num++;
        rdev_log_debug(MSG_LIZARD_ID, "idle thread created");
    }
    else
    {
        throw std::logic_error("error creating idle thread");
    }

    if(0 == pthread_create(&stats_th, NULL, &stats_loop_function, this))
    {
        threads_num++;
        rdev_log_debug(MSG_LIZARD_ID, "stats thread created");
    }
    else
    {
        throw std::logic_error("error creating stats thread");
    }
    rdev_log_info(MSG_LIZARD_ID, "%d internal threads created", threads_num);

    rdev_log_info(MSG_LIZARD_ID, "requested worker threads {easy: %d, hard: %d}", config.root.plugin.easy_threads, config.root.plugin.hard_threads);

    for(int i = 0; i < config.root.plugin.easy_threads; i++)
    {
        pthread_t th;
        int r = pthread_create(&th, NULL, &easy_loop_function, this);
        if(0 == r)
        {
            rdev_log_debug(MSG_LIZARD_ID, "easy thread created");
            easy_th.push_back(th);

            threads_num++;
        }
        else
        {
            char s[256];
            char buff[100];
            snprintf(s, 256, "error creating easy thread #%d : %s", i, strerror_r(r, buff, 100));
            throw std::logic_error(s);
        }
    }

    for(int i = 0; i < config.root.plugin.hard_threads; i++)
    {
        pthread_t th;
        int r = pthread_create(&th, NULL, &hard_loop_function, this);
        if(0 == r)
        {
            rdev_log_debug(MSG_LIZARD_ID, "hard thread created");
            hard_th.push_back(th);

            threads_num++;
        }
        else
        {
            char s[256];
            char buff[100];
            snprintf(s, 256, "error creating hard thread #%d : %s", i, strerror_r(r, buff, 100));
            throw std::logic_error(s);
        }
    }

    rdev_log_info(MSG_LIZARD_ID, "all worker threads created");
}

void lizard::server::join_threads()
{
    if(0 == threads_num)
    {
        return;
    }

    pthread_join(epoll_th,  0);
    threads_num--;

    pthread_join(idle_th,  0);
    threads_num--;

    pthread_join(stats_th,  0);
    threads_num--;

    for(size_t i = 0; i < easy_th.size(); i++)
    {
        rdev_log_debug(MSG_LIZARD_ID, "pthread_join(easy_th[%d], 0)", (int)i);
        pthread_join(easy_th[i], 0);
        threads_num--;
    }

    easy_th.clear();

    for(size_t i = 0; i < hard_th.size(); i++)
    {
        rdev_log_debug(MSG_LIZARD_ID, "pthread_join(hard_th[%d], 0)", (int)i);
        pthread_join(hard_th[i], 0);
        threads_num--;
    }

    hard_th.clear();

    rdev_log_debug(MSG_LIZARD_ID, "%d threads left", (int)threads_num);
}

void lizard::server::fire_all_threads()
{
    pthread_mutex_lock(&easy_proc_mutex);
    pthread_cond_broadcast(&easy_proc_cond);
    pthread_mutex_unlock(&easy_proc_mutex);

    pthread_mutex_lock(&hard_proc_mutex);
    pthread_cond_broadcast(&hard_proc_cond);
    pthread_mutex_unlock(&hard_proc_mutex);

    rdev_log_debug(MSG_LIZARD_ID, "fire_all_threads");
}

void lizard::server::epoll_send_wakeup()
{
    rdev_log_debug(MSG_LIZARD_ID, "epoll_send_wakeup()");
    char b[1] = {'w'};

    int ret;
    do
    {
        ret = write(epoll_wakeup_osock, b, 1);
        if(ret < 0 && errno != EINTR)
        {
            char buff[1024];
            rdev_log_error(MSG_LIZARD_ID, "epoll_send_wakeup():write failure: '%s'", strerror_r(errno, buff, 1024));
        }
    }
    while(ret < 0);
}

void lizard::server::epoll_recv_wakeup()
{
    rdev_log_debug(MSG_LIZARD_ID, "epoll_recv_wakeup()");
    char b[1024];

    int ret;
    do
    {
        ret = read(epoll_wakeup_isock, b, 1024);
        if(ret < 0 && errno != EAGAIN && errno != EINTR)
        {
            char buff[1024];
            rdev_log_error(MSG_LIZARD_ID, "epoll_recv_wakeup()::read failure: '%s'", strerror_r(errno, buff, 1024));
        }
    }
    while(ret == 1024 || errno == EINTR);
}

bool lizard::server::push_easy(http * el)
{
    bool res = false;

    pthread_mutex_lock(&easy_proc_mutex);

    size_t eq_sz = easy_queue.size(); 
    stats.report_easy_queue_len(eq_sz);

    if(config.root.plugin.easy_queue_limit == 0 || (eq_sz < (size_t)config.root.plugin.easy_queue_limit))
    {
        easy_queue.push_back(el);
        res = true;

        rdev_log_debug(MSG_LIZARD_ID, "push_easy %d", el->get_fd());

        pthread_cond_signal(&easy_proc_cond);
    }

    pthread_mutex_unlock(&easy_proc_mutex);

    return res;
}

bool lizard::server::pop_easy_or_wait(http** el)
{
    bool ret = false;

    pthread_mutex_lock(&easy_proc_mutex);

    size_t eq_sz = easy_queue.size();
    
    stats.report_easy_queue_len(eq_sz);

    if(eq_sz)
    {
        *el = easy_queue.front();

        rdev_log_debug(MSG_LIZARD_ID, "pop_easy %d", (*el)->get_fd());

        easy_queue.pop_front();

        ret = true;
    }
    else
    {
        rdev_log_debug(MSG_LIZARD_ID, "pop_easy : events empty");

        pthread_cond_wait(&easy_proc_cond, &easy_proc_mutex);
    }

    pthread_mutex_unlock(&easy_proc_mutex);

    return ret;
}

bool lizard::server::push_hard(http * el)
{
    bool res = false;

    pthread_mutex_lock(&hard_proc_mutex);

    size_t hq_sz = hard_queue.size();

    stats.report_hard_queue_len(hq_sz);

    if(config.root.plugin.hard_queue_limit == 0 || (hq_sz < (size_t)config.root.plugin.hard_queue_limit))
    {
        hard_queue.push_back(el);

        res = true;

        rdev_log_debug(MSG_LIZARD_ID, "push_hard %d", el->get_fd());

        pthread_cond_signal(&hard_proc_cond);
    }

    pthread_mutex_unlock(&hard_proc_mutex);

    return res;
}

bool lizard::server::pop_hard_or_wait(http** el)
{
    bool ret = false;

    pthread_mutex_lock(&hard_proc_mutex);
        
    size_t hq_sz = hard_queue.size(); 
    
    stats.report_hard_queue_len(hq_sz);

    if(hq_sz)
    {
        *el = hard_queue.front();

        rdev_log_debug(MSG_LIZARD_ID, "pop_hard %d", (*el)->get_fd());

        hard_queue.pop_front();

        ret = true;
    }
    else
    {
        rdev_log_debug(MSG_LIZARD_ID, "pop_hard : events empty");

        pthread_cond_wait(&hard_proc_cond, &hard_proc_mutex);
    }

    pthread_mutex_unlock(&hard_proc_mutex);

    return ret;
}
bool lizard::server::push_done(http * el)
{
    pthread_mutex_lock(&done_mutex);

    done_queue.push_back(el);

    stats.report_done_queue_len(done_queue.size());

    rdev_log_debug(MSG_LIZARD_ID, "push_done %d", el->get_fd());

    pthread_mutex_unlock(&done_mutex);

    epoll_send_wakeup();

    return true;
}

bool lizard::server::pop_done(http** el)
{
    bool ret = false;

    pthread_mutex_lock(&done_mutex);

        size_t dq_sz = done_queue.size(); 

    stats.report_done_queue_len(dq_sz);

    if(dq_sz)
    {
        *el = done_queue.front();

        rdev_log_debug(MSG_LIZARD_ID, "pop_done %d", (*el)->get_fd());

        done_queue.pop_front();

        ret = true;
    }

    pthread_mutex_unlock(&done_mutex);

    return ret;
}

//-----------------------------------------------------------------------------------------------------------

void lizard::server::load_config(const char *xml_in, std::string &pid_in)
{
    if(::access(xml_in, F_OK) < 0)
    {
        std::string s = "access('" + config.root.log_file_name + "') failed : ";
        char buff[1024];

                s += strerror_r(errno, buff, 1024); 

        throw std::logic_error(s);
    }

    config.clear();
    config.load_from_file(xml_in);
    config.check();

    if (pid_in.empty())
    {
        pid_in = config.root.pid_file_name;
    }

    rdev_log_close(MSG_LIZARD_ID);

    int res = rdev_log_sstr(MSG_LIZARD_ID,
                         config.root.log_file_name.c_str(),
                           config.root.log_level.c_str());
    if(res < 0)
    {
        std::string s = "logger init from ('" + config.root.log_file_name + "','" + config.root.log_level + "') failed: ";
        char buff[1024];

        s += strerror_r(errno, buff, 1024);
        
        throw std::logic_error(s);
    }

    rdev_log_close(MSG_LIZARD_ACCESS_LOG_ID);

    if(!config.root.access_log_file_name.empty())
    {
        /* 
         * bachan :
         *
         * access log entries are being written in RDEV_LOG_ACCESS level,
         * RDEV_LOG_INFO (or "info") is obsoleted interface routine.
         * 
         */

        int r = rdev_log_sacc(MSG_LIZARD_ACCESS_LOG_ID,
                        config.root.access_log_file_name.c_str());

        if(r < 0)
        {
            std::string s = "access_log init from " + config.root.access_log_file_name + " failed: ";
            char buff[1024];

            s += strerror_r(errno, buff, 1024);
            throw std::logic_error(s);
        }
    }
}

void lizard::server::prepare()
{
    srv_callback.init(this);

    factory.load_module(config.root.plugin, &srv_callback);

    epoll_sock = init_epoll();

    //----------------------------
    //add incoming sock

     incoming_sock = lz_utils::add_listener(config.root.plugin.ip.c_str(), config.root.plugin.port.c_str(), LISTEN_QUEUE_SZ);
        if(-1 != incoming_sock)
    {
            rdev_log_info(MSG_LIZARD_ID, "lizard is bound to %s:%s", config.root.plugin.ip.c_str(), config.root.plugin.port.c_str());
    }

    lz_utils::set_nonblocking(incoming_sock);
    add_epoll_action(incoming_sock, EPOLL_CTL_ADD, EPOLLIN);

    //----------------------------
    //add stats sock

    stats_sock = lz_utils::add_listener(config.root.stats.ip.c_str(), config.root.stats.port.c_str());
    if(-1 != stats_sock)
    {
        rdev_log_info(MSG_LIZARD_ID, "lizard statistics is bound to %s:%s", config.root.stats.ip.c_str(), config.root.stats.port.c_str());
    }

    lz_utils::set_socket_timeout(stats_sock, 50000);

    //----------------------------
    //add epoll wakeup fd

    int pipefd[2];
    if(::pipe(pipefd) == -1)
    {
        char buff[1024];
        throw std::logic_error((std::string)"server::prepare():pipe() failed : " + strerror_r(errno, buff, 1024));
    }

    epoll_wakeup_osock = pipefd[1];
    epoll_wakeup_isock = pipefd[0];

    lz_utils::set_nonblocking(epoll_wakeup_isock);
    add_epoll_action(epoll_wakeup_isock, EPOLL_CTL_ADD, EPOLLIN | EPOLLET);

}

void lizard::server::finalize()
{
    if(-1 != incoming_sock)
    {
        lz_utils::close_connection(incoming_sock);
        incoming_sock = -1;
    }

    if(-1 != stats_sock)
    {
        lz_utils::close_connection(stats_sock);
        stats_sock = -1;
    }

    if(-1 != epoll_wakeup_isock)
    {
        close(epoll_wakeup_isock);
        epoll_wakeup_isock = -1;
    }

    if(-1 != epoll_wakeup_osock)
    {
        close(epoll_wakeup_osock);
        epoll_wakeup_osock = -1;
    }

    if(-1 != epoll_sock)
    {
        close(epoll_sock);
        epoll_sock = -1;
    }

    factory.unload_module();
}

//-----------------------------------------------------------------------------------------------------------

int lizard::server::init_epoll()
{
    rdev_log_debug(MSG_LIZARD_ID, "init_epoll");

    int res = epoll_create(HINT_EPOLL_SIZE);
    if(-1 == res)
    {
        char buff[1024];
        throw std::logic_error((std::string)"epoll_create : " + strerror_r(errno, buff, 1024));
    }

    rdev_log_debug(MSG_LIZARD_ID, "epoll inited: %d", res);

    return res;
}

void lizard::server::add_epoll_action(int fd, int action, uint32_t mask)
{
    rdev_log_debug(MSG_LIZARD_ID, "add_epoll_action %d", fd);

    struct epoll_event evt;
    memset(&evt, 0, sizeof(evt));

    evt.events = mask;
    evt.data.fd = fd;

    int r;
    do
    {
        r = epoll_ctl(epoll_sock, action, fd, &evt);
    }
    while(r < 0 && errno == EINTR);

    if (-1 == r)
    {
        char buff[1024];
        std::string err_str = "add_epoll_action:epoll_ctl(";

        switch(action)
        {
            case EPOLL_CTL_ADD:
                err_str += "EPOLL_CTL_ADD";
                break;
            case EPOLL_CTL_MOD:
                err_str += "EPOLL_CTL_MOD";
                break;
            case EPOLL_CTL_DEL:
                err_str += "EPOLL_CTL_DEL";
                break;
        }
        err_str += ", " + events2string(evt.events) + ") : ";
        err_str += strerror_r(errno, buff, 1024);

        throw std::logic_error(err_str);
    }
}

void lizard::server::epoll_processing_loop()
{
    http * done_task = 0;
    while(pop_done(&done_task))
    {
        done_task->unlock();

        if(-1 != done_task->get_fd())
        {
            rdev_log_debug(MSG_LIZARD_ID, "%d is still alive", done_task->get_fd());
            process(done_task);
        }
        else
        {
            rdev_log_debug(MSG_LIZARD_ID, "%d is already dead while travelling through queues", done_task->get_fd());
            fds.release(done_task);
        }
    }

    int nfds = 0;
    do
    {
        nfds = epoll_wait(epoll_sock, events, EPOLL_EVENTS, fds.min_timeout()/*EPOLL_TIMEOUT*/);
    }
    while(nfds == -1 && (errno == EINTR || errno == EAGAIN));

    if(-1 == nfds)
    {
        char buff[1024];
        throw std::logic_error((std::string)"epoll_wait : " + strerror_r(errno, buff, 1024));
    }

    if(nfds)
    {
        if(nfds > 1)
        {
            rdev_log_debug(MSG_LIZARD_ID, "==  epoll_processing %d messages ==", nfds);
        }
        else
        {
            rdev_log_debug(MSG_LIZARD_ID, "==  epoll_processing 1 message ==");
        }

        //rdev_log_info(MSG_LIZARD_ID, "==  %d messages ==", nfds);
    }

    for(int i = 0; i < nfds; i++)
    {
        if(events[i].data.fd == epoll_wakeup_isock)
        {
            epoll_recv_wakeup();
        }
        else if(events[i].data.fd == incoming_sock)
        {
            struct in_addr ip;

            int client = lz_utils::accept_new_connection(incoming_sock, ip);

            if(client >= 0)
            {
                rdev_log_debug(MSG_LIZARD_ID, "accept_new_connection: %d from %s", client, inet_ntoa(ip));

                lz_utils::set_nonblocking(client);

                if(true == fds.create(client, ip))
                {
                    add_epoll_action(client, EPOLL_CTL_ADD, EPOLLIN | EPOLLOUT/* | EPOLLRDHUP*/ | EPOLLET);
                }
                else
                {
                    rdev_log_error(MSG_LIZARD_ID, "ERROR: fd[%d] is still in list of fds", client);
                }
            }
        }
        else
        {
            process_event(events[i]);
        }
    }

    fds.kill_oldest(1000 * config.root.plugin.connection_timeout);

    stats.process();

    if(rotate)
    {
        rdev_log_rotate(MSG_LIZARD_ID);
        rdev_log_rotate(MSG_LIZARD_ACCESS_LOG_ID);

        rotate = 0;
    }
}

bool lizard::server::process_event(const epoll_event& ev)
{
    rdev_log_debug(MSG_LIZARD_ID, "query event: %s", events2string(ev).c_str());

    http * con = fds.acquire(ev.data.fd);

    if(con)
    {
        if(ev.events & (EPOLLHUP | EPOLLERR))
        {
            rdev_log_debug(MSG_LIZARD_ID, "closing connection: got HUP/ERR!");
            con->set_rdeof();
            con->set_wreof();

            fds.del(ev.data.fd);
        }
        else
        {
                        if(ev.events & EPOLLIN)
                {
                con->allow_read();
                   }

            if(ev.events & EPOLLOUT)
            {
                con->allow_write();
            }


            /*if(ev.events & EPOLLRDHUP)
            {
                rdev_log_debug(MSG_LIZARD_ID, "push_event: RDHUP!");

                con->set_rdeof();
            }*/

            process(con);
        }
    }
    else
    {
        rdev_log_error(MSG_LIZARD_ID, "ERROR: process_event: unregistered fd in epoll set: %d", ev.data.fd);
    }

    return true;
}

bool lizard::server::process(http * con)
{
    if(!con->is_locked())
    {
        con->process();

        if(con->state() == http::sReadyToHandle)
        {
            if(!config.root.access_log_file_name.empty())
            {
                /*
                 * bachan :
                 *
                 * You can use rdev_log_access or rdev_log_message(YOUR_ID, RDEV_LOG_ACCESS, ...).
                 * Using another log levels with log, that was opened as access-log is defined as
                 * incorrect, 'cause I can't see any usability of log-levels in access-log-abstraction.
                 *
                 */

                rdev_log_access(MSG_LIZARD_ACCESS_LOG_ID, "%s|%s?%s|", inet_ntoa(con->get_request_ip()),
                    con->get_request_uri_path(), con->get_request_uri_params());
            }

            rdev_log_debug(MSG_LIZARD_ID, "push_easy(%d)", con->get_fd());

            con->lock();

            if(false == push_easy(con))
            {
                rdev_log_debug(MSG_LIZARD_ID, "easy queue full: easy_queue_size == %d", config.root.plugin.easy_queue_limit);

                con->set_response_status(503);
                con->set_response_header("Content-type", "text/plain");
                con->append_response_body("easy queue filled!", strlen("easy queue filled!"));

                push_done(con);
            }
        }
        else if(con->state() == http::sDone || con->state() == http::sUndefined)
        {
            rdev_log_debug(MSG_LIZARD_ID, "%d is done, closing write side of connection", con->get_fd());

            fds.del(con->get_fd());
        }
    }

    return true;
}

void lizard::server::easy_processing_loop()
{
    lizard::plugin * plugin = factory.get_plugin();

    //lizard::statistics::reporter easy_reporter(stats, lizard::statistics::reporter::repEasy);

    http * task = 0;

    if(pop_easy_or_wait(&task))
    {
        rdev_log_debug(MSG_LIZARD_ID, "lizard::easy_loop_function.fd = %d", task->get_fd());

        switch(plugin->handle_easy(task))
        {
        case plugin::rSuccess:

            rdev_log_debug(MSG_LIZARD_ID, "easy_loop: processed %d", task->get_fd());

            push_done(task);

            break;

        case plugin::rHard:

            rdev_log_debug(MSG_LIZARD_ID, "easy thread -> hard thread");

            if(config.root.plugin.hard_threads)
            {
                bool ret = push_hard(task);
                if(false == ret)
                {
                    rdev_log_debug(MSG_LIZARD_ID, "hard queue full: hard_queue_size == %d", config.root.plugin.hard_queue_limit);

                    task->set_response_status(503);
                    task->set_response_header("Content-type", "text/plain");
                    task->append_response_body("hard queue filled!", strlen("hard queue filled!"));

                    push_done(task);
                }
            }
            else
            {
                rdev_log_error(MSG_LIZARD_ID, "easy-thread tried to enqueue hard-thread, but config::plugin::hard_threads = 0");

                task->set_response_status(503);
                task->set_response_header("Content-type", "text/plain");
                task->append_response_body("easy loop error", strlen("easy loop error"));

                push_done(task);
            }

            break;

        case plugin::rError:

            rdev_log_error(MSG_LIZARD_ID, "easy thread reports error");

            task->set_response_status(503);
            task->set_response_header("Content-type", "text/plain");
            task->append_response_body("easy loop error", strlen("easy loop error"));

            push_done(task);

            break;
        }

        //easy_reporter.commit();
    }
}

void lizard::server::hard_processing_loop()
{
     lizard::plugin * plugin = factory.get_plugin();

//    lizard::statistics::reporter hard_reporter(stats, lizard::statistics::reporter::repHard);

    http * task = 0;

    if(pop_hard_or_wait(&task))
    {

        rdev_log_debug(MSG_LIZARD_ID, "lizard::hard_loop_function.fd = %d", task->get_fd());

        switch(plugin->handle_hard(task))
        {
        case plugin::rSuccess:

            rdev_log_debug(MSG_LIZARD_ID, "hard_loop: processed %d", task->get_fd());

            push_done(task);

            break;

        case plugin::rHard:
        case plugin::rError:

            rdev_log_error(MSG_LIZARD_ID, "hard_loop reports error");

            task->set_response_status(503);
            task->set_response_header("Content-type", "text/plain");
            task->append_response_body("hard loop error", strlen("hard loop error"));

            push_done(task);

            break;
        }

        //easy_reporter.commit();
    }
}

void lizard::server::idle_processing_loop()
{
    if(0 == config.root.plugin.idle_timeout)
    {
        rdev_log_debug(MSG_LIZARD_ID, "idle_loop_function: timing once");

        factory.idle();

        while(!quit && !hup)
        {
            sleep(1);
        }
    }
    else
    {
        rdev_log_debug(MSG_LIZARD_ID, "idle_loop_function: timing every %d(ms)", (int)config.root.plugin.idle_timeout);

        long secs = (config.root.plugin.idle_timeout * 1000000LLU) / 1000000000LLU;
        long nsecs = (config.root.plugin.idle_timeout * 1000000LLU) % 1000000000LLU;

        while(!quit && !hup)
        {
            factory.idle();

            lz_utils::uwait(secs, nsecs);
        }
    }
}


//--------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------

void *lizard::epoll_loop_function(void *ptr)
{
    lizard::server *srv = (lizard::server *) ptr;

    try
    {
        while (!quit && !hup)
        {
            srv->epoll_processing_loop();
        }
    }
    catch (const std::exception &e)
    {
        quit = 1;
        rdev_log_crit(MSG_LIZARD_ID, "epoll_loop: exception: %s", e.what());
    }

    srv->fire_all_threads();
    pthread_exit(NULL);
}

//--------------------------------------------------------------------------------------------------------

void *lizard::easy_loop_function(void *ptr)
{
    lizard::server *srv = (lizard::server *) ptr;

    try
    {
        while (!quit && !hup)
        {
            srv->easy_processing_loop();
        }
    }
    catch (const std::exception &e)
    {
        quit = 1;
        rdev_log_crit(MSG_LIZARD_ID, "easy_loop: exception: %s", e.what());
    }

    srv->fire_all_threads();
    pthread_exit(NULL);
}

//--------------------------------------------------------------------------------------------------------

void *lizard::hard_loop_function(void *ptr)
{
     lizard::server *srv = (lizard::server *) ptr;

    try
    {
        while (!quit && !hup)
        {
             srv->hard_processing_loop();
        }
    }
    catch (const std::exception &e)
    {
        quit = 1;
        rdev_log_crit(MSG_LIZARD_ID, "hard_loop: exception: %s", e.what());
    }

    srv->fire_all_threads();
    pthread_exit(NULL);
}

//--------------------------------------------------------------------------------------------------------

void *lizard::idle_loop_function(void *ptr)
{
    lizard::server *srv = (lizard::server *) ptr;

    try
    {
        while (!quit && !hup)
        {
             srv->idle_processing_loop();
        }
    }
    catch (const std::exception &e)
    {
        quit = 1;
        rdev_log_crit(MSG_LIZARD_ID, "idle_loop: exception: %s", e.what());
    }

    srv->fire_all_threads();
    pthread_exit(NULL);
}

//--------------------------------------------------------------------------------------------------------

void *lizard::stats_loop_function(void *ptr)
{
    lizard::server *srv = (lizard::server *) ptr;
    lizard::http stats_parser;

    try
    {
        while (!quit && !hup)
        {
            if(srv->stats_sock != -1)
            {
                struct in_addr ip;
                int stats_client = lz_utils::accept_new_connection(srv->stats_sock, ip);

                if(stats_client >= 0)
                {

                    lz_utils::set_socket_timeout(stats_client, 50000);

                    rdev_log_debug(MSG_LIZARD_ID, "stats: accept_new_connection: %d from %s", stats_client, inet_ntoa(ip));

                    stats_parser.init(stats_client, ip);

                    while(true)
                    {
                        stats_parser.allow_read();
                        stats_parser.allow_write();

                        stats_parser.process();

                        if(stats_parser.state() == http::sReadyToHandle)
                        {
                            if(!srv->config.root.access_log_file_name.empty())
                            {
                                rdev_log_access(MSG_LIZARD_ACCESS_LOG_ID, "%s|%s?%s|stats",
                                        inet_ntoa(stats_parser.get_request_ip()),
                                        stats_parser.get_request_uri_path(),
                                        stats_parser.get_request_uri_params());
                            }


                            stats_parser.set_response_status(200);
                            stats_parser.set_response_header("Content-type", "text/plain");

                            std::string resp = "<lizard_stats>\n";

                            //------------------------------------------------------------------------------------
                            char buff[1024];

                                                        time_t up_time = time(0) - srv->start_time;

                                                        snprintf(buff, 1024, "\t<lizard_version>"LIZARD_STR_VERSION"</lizard_version>\n\t<uptime>%d</uptime>\n", (int)up_time);
                                                        resp += buff;

                                                        snprintf(buff, 1024, "\t<rps>%.4f</rps>\n", stats.get_rps());
                                                        resp += buff;
                                                        
                            snprintf(buff, 1024, "\t<fd_count>%d</fd_count>\n", (int)srv->fds.fd_count());
                                                        resp += buff; 

                            snprintf(buff, 1024, "\t<queues>\n\t\t<easy>%d</easy>\n\t\t<max_easy>%d</max_easy>\n"
                                "\t\t<hard>%d</hard>\n\t\t<max_hard>%d</max_hard>\n\t\t<done>%d</done>\n"
                                "\t\t<max_done>%d</max_done>\n\t</queues>\n",
                                    (int)stats.easy_queue_len,
                                                                        (int)stats.easy_queue_max_len,
                                    (int)stats.hard_queue_len,
                                    (int)stats.hard_queue_max_len,
                                    (int)stats.done_queue_len,
                                    (int)stats.done_queue_max_len);
                            resp += buff;

                            snprintf(buff, 1024, "\t<conn_time>\n\t\t<min>%.4f</min>\n\t\t<avg>%.4f</avg>\n\t\t<max>%.4f</max>\n\t</conn_time>\n",
                                    stats.get_min_lifetime(), stats.get_mid_lifetime(), stats.get_max_lifetime());
                            resp += buff;

                            snprintf(buff, 1024, "\t<mem_allocator>\n\t\t<pages>%d</pages>\n\t\t<objects>%d</objects>\n\t</mem_allocator>\n",
                                    (int)stats.pages_in_http_pool, (int)stats.objects_in_http_pool);
                            resp += buff;


                            struct rusage usage;
                            ::getrusage(RUSAGE_SELF, &usage);

                            snprintf(buff, 1024, "\t<rusage>\n\t\t<utime>%d</utime>\n\t\t<stime>%d</stime>\n\t</rusage>\n",
                                    (int)usage.ru_utime.tv_sec, (int)usage.ru_stime.tv_sec);
                            resp += buff;

                            //------------------------------------------------------------------------------------
                            resp += "</lizard_stats>\n";

                            stats_parser.append_response_body(resp.data(), resp.size());

                        }
                        else if(stats_parser.state() == http::sDone)
                        {
                            rdev_log_debug(MSG_LIZARD_ID, "%d is done, closing write side of connection", stats_parser.get_fd());

                            break;
                        }
                    }

                    stats_parser.destroy();

                    stats.process();
                }
            }
            else
            {
                sleep(1);
            }
        }
    }
    catch (const std::exception &e)
    {
        quit = 1;
        rdev_log_crit(MSG_LIZARD_ID, "stats_loop: exception: %s", e.what());
    }

    srv->fire_all_threads();
    pthread_exit(NULL);
}

//--------------------------------------------------------------------------------------------------------

