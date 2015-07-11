#include "src/pch.h"
#include "src/core/common.h"
#include "ConnectionPool.h"
#include "DatabaseConnection.h"

namespace energonsoftware {

Logger& ConnectionPool::logger(Logger::instance("energonsoftware.core.database.ConnectionPool"));

ConnectionPool::ConnectionPool(size_t size) throw(ConnectionPoolError)
    : std::recursive_mutex(), _pool(), _size(size)
{
}

ConnectionPool::~ConnectionPool() noexcept
{
    if(_pool.size() != _size) {
        LOG_WARNING("Connection pool is missing resources!\n");
    }
    cleanup();
}

DatabaseConnection& ConnectionPool::acquire(bool block) throw(ConnectionPoolError)
{
    LOG_DEBUG("acquiring connection...\n");

    // wait for something to be in the pool
    while(block && _pool.empty()) {
        std::this_thread::sleep_for(std::chrono::microseconds(thread_sleep_time()));
        //std::thread::yield();
    }

    std::lock_guard<std::recursive_mutex> guard(*this);

    if(_pool.empty()) {
        throw ConnectionPoolError("Pool is empty!");
    }

    std::shared_ptr<DatabaseConnection> connection(_pool.front());
    _pool.pop_front();

    try {
        connection->connect();
    } catch(const DatabaseConnectionError& e) {
        throw ConnectionPoolError(std::string("Error acquiring database connection: ") + e.what());
    }

    LOG_DEBUG("acquired (" << connection->id() << ")\n");
    return *connection;
}

void ConnectionPool::release(std::shared_ptr<DatabaseConnection> connection)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    LOG_DEBUG("releasing (" << connection->id() << "), assuming disconnected\n");
    for(auto& conn : _pool) {
        if((*conn) == (*connection)) {
            // connection already in pool
            return;
        }
    }
    _pool.push_back(connection);
}

void ConnectionPool::cleanup() noexcept
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    for(auto& connection : _pool) {
        connection.reset();
    }
}

void ConnectionPool::push_connection(std::shared_ptr<DatabaseConnection> connection)
{
    std::lock_guard<std::recursive_mutex> guard(*this);
    _pool.push_back(connection);
}

}
