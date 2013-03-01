#include "src/pch.h"
#include "DatabaseConfiguration.h"

namespace energonsoftware {

DatabaseConfiguration::DatabaseConfiguration()
    : Configuration()
{
    set_default("database", "debug", "false");
    set_default("database", "port", "0");
    set_default("database", "pool", "0");
    set_default("database", "threads", "0");
    load_defaults();
}

DatabaseConfiguration::~DatabaseConfiguration() noexcept
{
}

void DatabaseConfiguration::validate() const throw(ConfigurationError)
{
    Configuration::validate();

    if(!is_int(get("database", "port"))) {
        throw ConfigurationError("Database port must be an integer!");
    }

    if(!is_int(get("database", "pool"))) {
        throw ConfigurationError("Database pool size must be an integer!");
    }

    if(!is_int(get("database", "threads"))) {
        throw ConfigurationError("Database thread count must be an integer!");
    }
}

}
