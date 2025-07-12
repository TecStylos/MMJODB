#pragma once

#include <memory>
#include <string>

#include "DBQuery.h"

class Database
{
public:
	Database(const std::string& filepath, bool allow_create);
	~Database();
public:
	DBQuery make_query(const std::string& sql_stmt_str);
private:
	sqlite3* m_db;
public:
	static bool has_instance();
	static std::shared_ptr<Database> get_instance();
	static void set_instance(std::shared_ptr<Database> db);
private:
	static std::shared_ptr<Database> s_singleton;
};