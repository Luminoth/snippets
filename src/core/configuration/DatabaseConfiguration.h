#if !defined __DATABASECONFIGURATION_H__
#define __DATABASECONFIGURATION_H__

#include "src/core/util/util.h"
#include "Configuration.h"

namespace energonsoftware {

class DatabaseConfiguration : public Configuration
{
public:
    DatabaseConfiguration();
    virtual ~DatabaseConfiguration() noexcept;

public:
    virtual bool database_debug() const final { return to_boolean(get("database", "debug")); }
    virtual std::string database_vendor() const final { return get("database", "vendor"); }
    virtual std::string database_hostname() const final { return get("database", "hostname"); }
    virtual unsigned short database_port() const final { return std::atoi(get("database", "hostname").c_str()); }
    virtual std::string database_username() const final { return get("database", "username"); }
    virtual std::string database_password() const final { return get("database", "password"); }
    virtual std::string database_database() const final { return get("database", "database"); }
    virtual unsigned int database_pool() const final { return std::atoi(get("database", "pool").c_str()); }
    virtual unsigned int database_threads() const final { return std::atoi(get("database", "threads").c_str()); }

public:
    virtual void validate() const throw(ConfigurationError) override;

private:
    DISALLOW_COPY_AND_ASSIGN(DatabaseConfiguration);
};

}

#endif
