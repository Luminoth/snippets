#if !defined __BASETHREAD_H__
#define __BASETHREAD_H__

namespace energonsoftware {

class ThreadPool;

class BaseThread
{
private:
    static Logger& logger;

public:
    explicit BaseThread(ThreadPool* const pool);
    explicit BaseThread(const std::string& name);

    // NOTE: this calls stop() on the thread
    virtual ~BaseThread() noexcept;

public:
    virtual std::string name() const final;

    virtual void start() final;

    virtual void quit() final { _quit = true; }

    // NOTE: this joins the thread
    virtual void stop() final;

    virtual bool should_quit() const final { return _quit; }

    // releases ownership of the thread (used by the thread pool)
    virtual std::shared_ptr<boost::thread> release() final;

    std::string str() const;

protected:
    // NOTE: be *extremely* careful when locking this
    // as a deadlock situation exists when the pool is stopped.
    // Use a unique_lock with try_lock() rather than a lock_guard to be safe
    virtual ThreadPool* pool() final { return _pool; }
    virtual const ThreadPool* pool() const final { return _pool; }

    // for non-pooled threads
    virtual void on_run() {}

private:
    void run();

private:
    ThreadPool* _pool;
    std::string _name;
    bool _quit;

    std::shared_ptr<boost::thread> _thread;
    bool _own_thread;

private:
    DISALLOW_COPY_AND_ASSIGN(BaseThread);
};

// override this and pass to ThreadPool.start() to create
// new threads of a specific type
class ThreadFactory
{
public:
    ThreadFactory();
    virtual ~ThreadFactory() noexcept;

public:
    virtual std::shared_ptr<BaseThread> new_thread(ThreadPool* pool=nullptr) const noexcept = 0;
};

}

#endif
