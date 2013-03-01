#if !defined __BASEJOB_H__
#define __BASEJOB_H__

namespace energonsoftware {

class BaseJob
{
public:
    explicit BaseJob(int priority=0)
        : _priority(priority)
    {
    }

    virtual ~BaseJob() noexcept
    {
    }

public:
    virtual int priority() const final { return _priority; }
    virtual void process_work() final { on_process_work(); }

protected:
    virtual void on_process_work() = 0;

private:
    int _priority;
};

}

#endif
