#if !defined __DATABASEOBJECT_H__
#define __DATABASEOBJECT_H__

#include "DatabaseConnection.h"

namespace energonsoftware {

class DatabaseObject
{
public:
    DatabaseObject() { }
    virtual ~DatabaseObject() throw() { }

public:
    bool save(std::shared_ptr<DatabaseConnection> connection) throw(DatabaseConnectionError)
    {
        if(on_save(connection)) {
            connection->commit();
            return true;
        }
        return false;
    }

public:
    virtual bool load(std::shared_ptr<DatabaseConnection> connection) throw(DatabaseConnectionError) { return false; }

protected:
    virtual bool on_save(std::shared_ptr<DatabaseConnection> connection) throw(DatabaseConnectionError) { return false; }
};

}

#endif
