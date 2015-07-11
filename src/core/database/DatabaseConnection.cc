#include "src/pch.h"
#include "src/core/configuration/DatabaseConfiguration.h"
#include "src/core/util/util.h"
#include "DatabaseConnection.h"
#include "ConnectionPool.h"

namespace energonsoftware {

Logger& DatabaseConnection::logger(Logger::instance("energonsoftware.core.database.DatabaseConnection"));

DatabaseConnection::DatabaseConnection(int64_t id, const DatabaseConfiguration& config, std::shared_ptr<ConnectionPool> pool) throw(DatabaseConnectionError)
    : std::recursive_mutex(), std::enable_shared_from_this<DatabaseConnection>(),
        _host(), _port(0), _id(id), _connected(false), _config(config), _pool(pool)
{
    std::string vendor(this->config().database_vendor());
    _host = this->config().database_hostname();
    _port = this->config().database_port();
}

DatabaseConnection::~DatabaseConnection() noexcept
{
}

void DatabaseConnection::connect() throw(DatabaseConnectionError)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    if(!connected()) {
        std::string database(config().database_database());
        std::string username(config().database_username());

        if(config().database_debug()) {
            LOG_DEBUG("Connecting (" << _id << ") to database '" << database << "' as user '" << username << "'...\n");
        }

        try {
            on_connect(database, username, config().database_password());
        } catch(const DatabaseConnectionError& e) {
            disconnect();
            throw e;
        }
    }
    _connected = true;
}

void DatabaseConnection::disconnect() throw(DatabaseConnectionError)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    if(config().database_debug()) {
        LOG_DEBUG("Closing database (" << _id << ") connection...\n");
    }

    _connected = false;
    if(_pool) _pool->release(shared_from_this());

    on_disconnect();
}

bool DatabaseConnection::begin_transaction() throw(DatabaseConnectionError)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    return on_begin_transaction();
}

bool DatabaseConnection::commit() throw(DatabaseConnectionError)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    return on_commit();
}

bool DatabaseConnection::rollback() throw(DatabaseConnectionError)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    return on_rollback();
}

bool DatabaseConnection::query(const std::string& query, std::pair<int, DatabaseResults>& results, bool silent) throw(DatabaseConnectionError)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    if(!connected()) {
        return false;
    }

    if(!silent && config().database_debug()) {
        LOG_DEBUG("Executing (" << _id << ") query: '" << query << "'...\n");
    }

    results.first = 0;

    double start = get_time();
    bool ret = on_query(query, results, silent);
    double end = get_time();

    if(!silent && config().database_debug()) {
        LOG_DEBUG("Query (" << _id << ") took " << end - start << " seconds\n");
    }

    return ret;
}

bool DatabaseConnection::query(const std::string& query, const std::vector<DatabaseValue>& params, std::pair<int, DatabaseResults>& results, bool silent) throw(DatabaseConnectionError)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    if(!connected()) {
        return false;
    }

    if(!silent && config().database_debug()) {
        LOG_DEBUG("Executing (" << _id << ") query: '" << query << "'...\n");
    }

    results.first = 0;

    double start = get_time();
    bool ret = on_query(query, params, results, silent);
    double end = get_time();

    if(!silent && config().database_debug()) {
        LOG_DEBUG("Query (" << _id << ") took " << end - start << " seconds\n");
    }

    return ret;
}

bool DatabaseConnection::execute(const std::string& query, bool silent) throw(DatabaseConnectionError)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    std::pair<int, DatabaseResults> results;
    return this->query(query, results, silent);
}

bool DatabaseConnection::execute(const std::string& query, const std::vector<DatabaseValue>& params, bool silent) throw(DatabaseConnectionError)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    std::pair<int, DatabaseResults> results;
    return this->query(query, params, results, silent);
}

DatabaseKey DatabaseConnection::insert_id() throw(DatabaseConnectionError)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    return on_insert_id();
}

void DatabaseConnection::escape(const std::string& from, std::string& to) throw(DatabaseConnectionError)
{
    std::lock_guard<std::recursive_mutex> guard(*this);

    on_escape(from, to);
}

}
