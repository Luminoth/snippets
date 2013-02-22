#if !defined __DATABASECONNECTIONFACTORY_H__
#define __DATABASECONNECTIONFACTORY_H__

#include "src/core/database/ConnectionPool.h"
#include "src/core/database/DatabaseError.h"

namespace energonsoftware {

class DatabaseConfiguration;
class DatabaseConnection;

class DatabaseConnectionFactory
{
public:
    // NOTE: *not* a shared pointer
    static std::shared_ptr<DatabaseConnection> new_connection(const DatabaseConfiguration& config, std::shared_ptr<ConnectionPool> pool=std::shared_ptr<ConnectionPool>()) throw(DatabaseError);
    static std::shared_ptr<ConnectionPool> new_pool(const DatabaseConfiguration& config) throw(ConnectionPoolError);

private:
    static Logger& logger;
    static long last_id;

private:
    DatabaseConnectionFactory();
    virtual ~DatabaseConnectionFactory();
};

}

#endif
