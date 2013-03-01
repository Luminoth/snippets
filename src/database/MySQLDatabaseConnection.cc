#include "src/pch.h"
#include <mysql/mysql.h>
#include "src/core/configuration/DatabaseConfiguration.h"
#include "MySQLDatabaseConnection.h"

namespace energonsoftware {

Logger& MySQLDatabaseConnection::logger(Logger::instance("energonsoftware.database.MySQLDatabaseConnection"));
size_t MySQLDatabaseConnection::connection_count = 0;
boost::recursive_mutex MySQLDatabaseConnection::init_mutex;

MySQLDatabaseConnection::MySQLDatabaseConnection(long id, const DatabaseConfiguration& config, std::shared_ptr<ConnectionPool> pool) throw(DatabaseConnectionError)
    : DatabaseConnection(id, config, pool), _mysql(nullptr)
{
    boost::lock_guard<boost::recursive_mutex> guard(init_mutex);

    if(!connection_count) {
        if(mysql_library_init(0, nullptr, nullptr)) {
            throw DatabaseError("Could not initialize MySQL!");
        }
    }
    connection_count++;
}

MySQLDatabaseConnection::~MySQLDatabaseConnection() noexcept
{
    boost::lock_guard<boost::recursive_mutex> guard(init_mutex);

    connection_count--;
    if(!connection_count) {
        mysql_library_end();
    }
}

void MySQLDatabaseConnection::on_connect(const std::string& database, const std::string& username, const std::string& password) throw(DatabaseConnectionError)
{
    _mysql = mysql_init(nullptr);
    if(!_mysql) {
        throw DatabaseConnectionError("Error initializing MySQL connection!");
    }

    if(!mysql_real_connect(_mysql, _host.c_str(), username.c_str(), password.c_str(), database.c_str(), _port, nullptr, 0)) {
        throw DatabaseConnectionError(std::string("Could not connect to MySQL host: ") + mysql_error(_mysql));
    }
}

void MySQLDatabaseConnection::on_disconnect() throw(DatabaseConnectionError)
{
    mysql_thread_end();
    mysql_close(_mysql);
    _mysql = nullptr;
}

bool MySQLDatabaseConnection::on_begin_transaction() throw(DatabaseConnectionError)
{
    return execute("start transaction;");
}

bool MySQLDatabaseConnection::on_query(const std::string& query, std::pair<int, DatabaseResults>& results, bool silent) throw(DatabaseConnectionError)
{
    if(mysql_real_query(_mysql, query.c_str(), query.length())) {
        return false;
    }

    return handle_result(results, silent);
}

bool MySQLDatabaseConnection::on_query(const std::string& query, const std::vector<DatabaseValue>& params, std::pair<int, DatabaseResults>& results, bool silent) throw(DatabaseConnectionError)
{
/* TODO: write meh */
    return false;
}

bool MySQLDatabaseConnection::on_commit() throw(DatabaseConnectionError)
{
    return mysql_commit(_mysql) == 0;
}

bool MySQLDatabaseConnection::on_rollback() throw(DatabaseConnectionError)
{
    return mysql_rollback(_mysql) == 0;
}

DatabaseKey MySQLDatabaseConnection::on_insert_id() throw(DatabaseConnectionError)
{
    return mysql_insert_id(_mysql);
}

void MySQLDatabaseConnection::on_escape(const std::string& from, std::string& to) throw(DatabaseConnectionError)
{
    boost::shared_array<char> buffer(new char[from.length() * 2 + 1]);
    mysql_real_escape_string(_mysql, buffer.get(), from.c_str(), from.length());
    to = std::string(buffer.get());
}

bool MySQLDatabaseConnection::handle_result(std::pair<int, DatabaseResults>& results, bool silent) throw(DatabaseConnectionError)
{
    MYSQL_RES* result = mysql_use_result(_mysql);
    if(result) {
        int field_count = mysql_num_fields(result);
        MYSQL_FIELD* fields = mysql_fetch_fields(result);

        MYSQL_ROW row;
        while((row = mysql_fetch_row(result))) {
            results.second.push_back(DatabaseResult());
            for(int i=0; i<field_count; ++i) {
                DatabaseValue& value(results.second[results.first][fields[i].name]);

/* TODO: improve this with the new db typing rather than forcing everything to be text */
                if(nullptr != row[i]) {
                    /*if(!silent && config().database_debug()) {
                        if(results.first < 5) {
                            LOG_DEBUG("MySQL Result: " << fields[i].name << " = " << row[i] << "\n");
                        } else if(results.first == 5) {
                            LOG_DEBUG("more than 5 results found, stopping printing\n");
                        }
                    }*/
                    value.type = DatabaseType::Text;
                    value.value = boost::any(static_cast<DatabaseText>(row[i]));
                } else {
                    /*if(!silent && config().database_debug()) {
                        if(results.first < 5) {
                            LOG_DEBUG("MySQL Result: " << fields[i].name << " = null\n");
                        } else if(results.first == 5) {
                            LOG_DEBUG("more than 5 results found, stopping printing\n");
                        }
                    }*/
                    value.type = DatabaseType::Null;
                }
            }
            results.first++;
        }
        mysql_free_result(result);
    } else {
        if(mysql_field_count(_mysql) == 0) {
            results.first = mysql_affected_rows(_mysql);
        }
    }

    return true;
}

}
