#pragma once

#include <memory>
#include <string>
#include <map>

#include "DBQuery.h"

class Database
{
public:
	struct TableInfo
	{
		std::string name;
		std::map<std::string, DBRow::Column::Type> columns; // column name -> column type
	};
public:
	Database(const std::string& filepath, bool allow_create);
	~Database();
public:
	DBQuery make_query(const std::string& sql_stmt_str);
public:
	std::vector<std::string> get_table_names();
	TableInfo get_table_info(const std::string& table_name);
private:
	sqlite3* m_db;
public:
	static bool has_instance();
	static std::shared_ptr<Database> get_instance();
	static void set_instance(std::shared_ptr<Database> db);
private:
	static std::shared_ptr<Database> s_singleton;
};