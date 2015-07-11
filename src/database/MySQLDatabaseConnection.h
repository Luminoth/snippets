#if !defined __MYSQLDATABASECONNECTION_H__
#define __MYSQLDATABASECONNECTION_H__

#if 0
#include "src/core/database/DatabaseConnection.h"

typedef struct st_mysql MYSQL;

namespace energonsoftware {

class MySQLDatabaseConnection : public DatabaseConnection
{
private:
    static Logger& logger;
    static size_t connection_count;
    static boost::recursive_mutex init_mutex;

public:
    virtual ~MySQLDatabaseConnection() noexcept;

private:
    virtual void on_connect(const std::string& database, const std::string& username, const std::string& password) throw(DatabaseConnectionError);
    virtual void on_disconnect() throw(DatabaseConnectionError);
    virtual bool on_begin_transaction() throw(DatabaseConnectionError);
    virtual bool on_query(const std::string& query, std::pair<int, DatabaseResults>& results, bool silent) throw(DatabaseConnectionError);
    virtual bool on_query(const std::string& query, const std::vector<DatabaseValue>& params, std::pair<int, DatabaseResults>& results, bool silent) throw(DatabaseConnectionError);
    virtual bool on_commit() throw(DatabaseConnectionError);
    virtual bool on_rollback() throw(DatabaseConnectionError);
    virtual DatabaseKey on_insert_id() throw(DatabaseConnectionError);
    virtual void on_escape(const std::string& from, std::string& to) throw(DatabaseConnectionError);

    bool handle_result(std::pair<int, DatabaseResults>& results, bool silent) throw(DatabaseConnectionError);

private:
    MYSQL* _mysql;

private:
    friend class DatabaseConnectionFactory;
    MySQLDatabaseConnection(long id, const DatabaseConfiguration& config, std::shared_ptr<ConnectionPool> pool=std::shared_ptr<ConnectionPool>()) throw(DatabaseConnectionError);

    MySQLDatabaseConnection();
    DISALLOW_COPY_AND_ASSIGN(MySQLDatabaseConnection);
};

}
#endif

#endif
