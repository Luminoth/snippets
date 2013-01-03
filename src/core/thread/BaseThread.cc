#include "src/pch.h"
#include "src/core/common.h"
#include "src/core/text/string_util.h"
#include "BaseJob.h"
#include "BaseThread.h"
#include "ThreadPool.h"

namespace energonsoftware {

Logger& BaseThread::logger(Logger::instance("energonsoftware.core.thread.BaseThread"));

BaseThread::BaseThread(ThreadPool* pool)
    : _pool(pool), _quit(false), _own_thread(false)
{
}

BaseThread::BaseThread(const std::string& name)
    : _pool(nullptr), _name(name), _quit(false), _own_thread(false)
{
}

BaseThread::~BaseThread() throw()
{
    stop();
}

std::string BaseThread::name() const
{
    if(_pool) {
        // name is the threadid when we're part of a pool
        //return to_string(pthread_self());

        std::stringstream ss;
        ss << _thread->get_id();
        return ss.str();
    }
    return _name;
}

void BaseThread::start()
{
    stop();
    _quit = false;
    _own_thread = true;

    _thread.reset(new boost::thread(boost::bind(&BaseThread::run, this)));
}

void BaseThread::stop()
{
    quit();
    if(_own_thread && _thread) {
        LOG_INFO("Joining thread '" << name() << "'...\n");
        _thread->join();
        LOG_DEBUG("Success!\n");

        _thread.reset();
        _own_thread = false;
    }
}

std::shared_ptr<boost::thread> BaseThread::release()
{
    _own_thread = false;
    return _thread;
}

std::string BaseThread::str() const
{
    std::stringstream ss;
    ss << "Thread ";
    if(_thread) {
        ss << _thread->get_id();
    } else {
        ss << "none";
    }
    ss << " Info:\n";

    ss << "Name: " << name() << "\n"
        << "Has thread: " << to_string(static_cast<bool>(_thread)) << "\n"
        << "Owns thread: " << to_string(_own_thread) << "\n"
        << "Has pool: " << to_string(_pool != nullptr) << "\n"
        << "Should quit: " << to_string(_quit);
    return ss.str();
}

void BaseThread::run()
{
    // NOTE: must use _name here because _thread isn't valid yet
    LOG_DEBUG("Waiting for thread '" << _name << "' to start...\n");
    while(!_thread) {}

    LOG_DEBUG("Running thread '" << name() << "'\n");
    //LOG_DEBUG(str() << "\n");

    boost::this_thread::interruption_enabled();
    while(!should_quit() && !boost::this_thread::interruption_requested()) {
        try {
            if(pool()) {
                boost::unique_lock<boost::recursive_mutex> guard(*(pool()), boost::try_to_lock);
                if(guard.owns_lock()) {
                    if(pool()->has_work()) {
                        std::shared_ptr<BaseJob> job(pool()->pop_work());
                        job->process_work();
                    }
                }
            } else {
               on_run();
            }

            boost::this_thread::sleep(boost::posix_time::microseconds(thread_sleep_time()));
            //boost::this_thread::yield();
        } catch(const boost::thread_interrupted&) {
            quit();
        } catch(const std::exception& e) {
            LOG_CRITICAL("Thread '" << name() << "' caught unhandled exception: " << e.what() << "\n");

            // TODO: this used to pass to main() in python... can we do that here?
            quit();
        } catch(...) {
            LOG_CRITICAL("Thread '" << name() << "' caught unhandled exception! Quitting...\n");

            // TODO: this used to pass to main() in python... can we do that here?
            quit();
        }
    }

    LOG_DEBUG("Thread '" << name() << "' is quitting...\n");
    //LOG_DEBUG(str() << "\n");
}

ThreadFactory::ThreadFactory()
{
}

ThreadFactory::~ThreadFactory() throw()
{
}

}
