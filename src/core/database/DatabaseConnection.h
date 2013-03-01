#if !defined __DATABASECONNECTION_H__
#define __DATABASECONNECTION_H__

#include "DatabaseError.h"

namespace energonsoftware {

class DatabaseConnectionError : public DatabaseError
{
public:
    explicit DatabaseConnectionError(const std::string& what) noexcept : DatabaseError(what) { }
    virtual ~DatabaseConnectionError() noexcept {}
};

enum class DatabaseType
{
    Null,
    Integer,
    Real,
    Text,
    Blob,
};

struct DatabaseValue
{
    DatabaseType type;
    boost::any value;

    DatabaseValue()
        : type(DatabaseType::Null), value()
    {
    }

    virtual ~DatabaseValue() noexcept
    {
    }

    operator bool() const { return type != DatabaseType::Null; }
    bool operator!() const { return type == DatabaseType::Null; }
};

typedef uint64_t DatabaseKey;
typedef int32_t DatabaseInteger;
typedef double DatabaseReal;
typedef std::string DatabaseText;
typedef std::vector<unsigned char> DatabaseBlob;
typedef std::unordered_map<std::string, DatabaseValue> DatabaseResult;
typedef std::vector<DatabaseResult> DatabaseResults;

const DatabaseKey INVALID_DATABASE_KEY = 0;

class ConnectionPool;
class DatabaseConfiguration;

class DatabaseConnection : public boost::recursive_mutex, public std::enable_shared_from_this<DatabaseConnection>
{
private:
    static Logger& logger;

public:
    DatabaseConnection(int64_t id, const DatabaseConfiguration& config, std::shared_ptr<ConnectionPool> pool=std::shared_ptr<ConnectionPool>()) throw(DatabaseConnectionError);
    virtual ~DatabaseConnection() noexcept;

public:
    // connects to the database
    // does nothing if already connected
    virtual void connect() throw(DatabaseConnectionError) final;

    // disconnects from the database and releases the connection if it's owned by a pool
    virtual void disconnect() throw(DatabaseConnectionError) final;

    // starts/ends a transaction
    virtual bool begin_transaction() throw(DatabaseConnectionError) final;
    virtual bool commit() throw(DatabaseConnectionError) final;
    virtual bool rollback() throw(DatabaseConnectionError) final;

    // executes a select query and stores the result in results
    // results.first is the number of affected rows for non-SELECT queries or the number of results encountered
    // results.second is the result set for SELECT queries
    // returns false if the query was not executed
    // TODO: this doesn't handle multi-result queries
    virtual bool query(const std::string& query, std::pair<int, DatabaseResults>& results, bool silent=false) throw(DatabaseConnectionError) final;
    virtual bool query(const std::string& query, const std::vector<DatabaseValue>& params, std::pair<int, DatabaseResults>& results, bool silent=false) throw(DatabaseConnectionError) final;
    virtual bool execute(const std::string& query, bool silent=false) throw(DatabaseConnectionError) final;
    virtual bool execute(const std::string& query, const std::vector<DatabaseValue>& params, bool silent=false) throw(DatabaseConnectionError) final;

    // returns the id of the last insert
    virtual DatabaseKey insert_id() throw(DatabaseConnectionError) final;

    // escapes input
    virtual void escape(const std::string& from, std::string& to) throw(DatabaseConnectionError) final;

    virtual int64_t id() const final { return _id; }

    virtual bool connected() const final { return _connected; }

public:
    // only works for connections owned by a pool
    virtual bool operator==(const DatabaseConnection& rhs) const final { return _id == rhs._id; }

protected:
    virtual const DatabaseConfiguration& config() final { return _config; }

protected:
    // override these
    virtual void on_connect(const std::string& database, const std::string& username, const std::string& password) throw(DatabaseConnectionError) = 0;
    virtual void on_disconnect() throw(DatabaseConnectionError) = 0;
    virtual bool on_begin_transaction() throw(DatabaseConnectionError) = 0;
    virtual bool on_query(const std::string& query, std::pair<int, DatabaseResults>& results, bool silent) throw(DatabaseConnectionError) = 0;
    virtual bool on_query(const std::string& query, const std::vector<DatabaseValue>& params, std::pair<int, DatabaseResults>& results, bool silent) throw(DatabaseConnectionError) = 0;
    virtual bool on_commit() throw(DatabaseConnectionError) = 0;
    virtual bool on_rollback() throw(DatabaseConnectionError) = 0;
    virtual DatabaseKey on_insert_id() throw(DatabaseConnectionError) = 0;
    virtual void on_escape(const std::string& from, std::string& to) throw(DatabaseConnectionError) { to = from; }

protected:
    std::string _host;
    unsigned short _port;

private:
    int64_t _id;
    bool _connected;
    const DatabaseConfiguration& _config;
    std::shared_ptr<ConnectionPool> _pool;

private:
    DatabaseConnection();
    DISALLOW_COPY_AND_ASSIGN(DatabaseConnection);
};

}

#endif
