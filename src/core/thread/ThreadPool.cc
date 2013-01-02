#include "src/pch.h"
#include "ThreadPool.h"
#include "BaseThread.h"

namespace energonsoftware {

Logger& ThreadPool::logger(Logger::instance("energonsoftware.core.thread.ThreadPool"));

ThreadPool::ThreadPool(size_t size)
    : boost::recursive_mutex(), _size(size), _running(false)
{
}

ThreadPool::~ThreadPool() throw()
{
    stop();
}

void ThreadPool::start(const ThreadFactory& factory)
{
    boost::lock_guard<boost::recursive_mutex> guard(*this);

    stop();

    if(_size == 0) {
        return;
    }

    LOG_INFO("Initializing " << _size << " threads...\n");
    for(size_t i=0; i<_size; ++i) {
        std::shared_ptr<BaseThread> thread(factory.new_thread(this));
        thread->start();

        std::shared_ptr<boost::thread> native(thread->release());
        _thread_group.add_thread(native.get());
        _threads.push_back(native);
    }
    _running = true;
}

void ThreadPool::push_work(std::shared_ptr<BaseJob> job)
{
    boost::lock_guard<boost::recursive_mutex> guard(*this);

    _work.push(job);
}

bool ThreadPool::has_work()
{
    boost::lock_guard<boost::recursive_mutex> guard(*this);

    return !_work.empty();
}

std::shared_ptr<BaseJob> ThreadPool::pop_work()
{
    boost::lock_guard<boost::recursive_mutex> guard(*this);

    std::shared_ptr<BaseJob> ret(_work.top());
    _work.pop();
    return ret;
}

void ThreadPool::stop()
{
    boost::lock_guard<boost::recursive_mutex> guard(*this);

    if(running()) {
        LOG_INFO("Waiting for " << _size << " threads to finish...\n");
        _thread_group.interrupt_all();
        _thread_group.join_all();
        LOG_DEBUG("Finished!\n");
    }
    _running = false;
}

}
