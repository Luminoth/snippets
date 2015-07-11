#include "src/pch.h"
#include "src/core/configuration/DatabaseConfiguration.h"
#include "src/core/database/DatabaseConnection.h"
#include "DatabaseConnectionFactory.h"
//#include "MySQLDatabaseConnection.h"
//#include "SQLiteDatabaseConnection.h"

namespace energonsoftware {

Logger& DatabaseConnectionFactory::logger(Logger::instance("energonsoftware.database.DatabaseConnectionFactory"));
long DatabaseConnectionFactory::last_id = 0;

std::shared_ptr<DatabaseConnection> DatabaseConnectionFactory::new_connection(const DatabaseConfiguration& config, std::shared_ptr<ConnectionPool> pool) throw(DatabaseError)
{
    long id = pool ? ++last_id : -1;
    std::string vendor(config.database_vendor());

    std::string scratch(boost::algorithm::to_lower_copy(vendor));
    /*if(scratch == "mysql") {
        return std::shared_ptr<DatabaseConnection>(new MySQLDatabaseConnection(id, config, pool));
    }*/ /*else if(scratch == "sqlite") {
        return new SQLiteDatabaseConnection(id, config, pool);
    }*/
    throw DatabaseError("Vendor not supported: " + vendor);
}

std::shared_ptr<ConnectionPool> DatabaseConnectionFactory::new_pool(const DatabaseConfiguration& config) throw(ConnectionPoolError)
{
    std::shared_ptr<ConnectionPool> pool(new ConnectionPool(config.database_pool()));
    try {
        for(size_t i=0; i<pool->size(); ++i) {
            pool->push_connection(new_connection(config, pool));
        }
    } catch(const DatabaseConnectionError& e) {
        throw ConnectionPoolError(e.what());
    }
    return pool;
}

}
