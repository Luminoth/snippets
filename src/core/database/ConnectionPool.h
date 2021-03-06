#if !defined __CONNECTIONPOOL_H__
#define __CONNECTIONPOOL_H__

#include "DatabaseError.h"

namespace energonsoftware {

class ConnectionPoolError : public DatabaseError
{
public:
    explicit ConnectionPoolError(const std::string& what) noexcept : DatabaseError(what) {}
    virtual ~ConnectionPoolError() noexcept {}
};

class DatabaseConnection;

// TODO: this should *not* inherit from std::recursive_mutex
class ConnectionPool : public std::recursive_mutex
{
private:
    static Logger& logger;

public:
    virtual ~ConnectionPool() noexcept;

public:
    // acquires a connection
    DatabaseConnection& acquire(bool block=true) throw(ConnectionPoolError);

    // releases a connection back to the pool
    // NOTE: this assumes the connection has been disconnected
    void release(std::shared_ptr<DatabaseConnection> connection);

    size_t size() { return _size; }

private:
    void cleanup() noexcept;
    void push_connection(std::shared_ptr<DatabaseConnection> connection);

private:
    std::deque<std::shared_ptr<DatabaseConnection>> _pool;
    size_t _size;

private:
    friend class DatabaseConnectionFactory;
    explicit ConnectionPool(size_t size) throw(ConnectionPoolError);

    ConnectionPool() = delete;
    DISALLOW_COPY_AND_ASSIGN(ConnectionPool);
};

}

#endif
