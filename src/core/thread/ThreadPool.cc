#include "src/pch.h"
#include "ThreadPool.h"
#include "BaseThread.h"

namespace energonsoftware {

Logger& ThreadPool::logger(Logger::instance("energonsoftware.core.thread.ThreadPool"));

ThreadPool::ThreadPool(size_t size)
    : std::recursive_mutex(), _size(size), _threads(), _running(false), _work()
{
}

ThreadPool::~ThreadPool() noexcept
{
    stop();
}

void ThreadPool::start(const ThreadFactory& factory)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    stop();

    if(_size == 0) {
        return;
    }

    LOG_INFO("Initializing " << _size << " threads...\n");
    for(size_t i=0; i<_size; ++i) {
        std::shared_ptr<BaseThread> thread(factory.new_thread(this));
        thread->start();

        std::shared_ptr<std::thread> native(thread->release());
        _threads.push_back(native);
    }
    _running = true;
}

void ThreadPool::push_work(std::shared_ptr<BaseJob> job)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    _work.push(job);
}

bool ThreadPool::has_work()
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    return !_work.empty();
}

std::shared_ptr<BaseJob> ThreadPool::pop_work()
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    std::shared_ptr<BaseJob> ret(_work.top());
    _work.pop();
    return ret;
}

void ThreadPool::stop()
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    if(running()) {
        LOG_INFO("Waiting for " << _size << " threads to finish...\n");
        for(auto thread : _threads) {
            thread->join();
        }
        LOG_DEBUG("Finished!\n");
    }
    _running = false;
}

}
