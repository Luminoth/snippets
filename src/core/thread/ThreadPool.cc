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
        BaseThread* thread = factory.new_thread(this);
        thread->start();
        _threads.add_thread(thread->release());
    }
    _running = true;
}

void ThreadPool::push_work(BaseJob* job)
{
    boost::lock_guard<boost::recursive_mutex> guard(*this);

    _work.push(std::shared_ptr<BaseJob>(job));
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
        _threads.interrupt_all();
        _threads.join_all();
        LOG_DEBUG("Finished!\n");
    }
    _running = false;
}

}
