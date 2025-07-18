#include "stdafx.h"
#include "Database.h"

#include "Strings.h"

std::shared_ptr<Database> Database::s_singleton = nullptr;

Database::Database(const std::string& filepath, bool allow_create)
{
	int open_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX;
	if (allow_create)
		open_flags |= SQLITE_OPEN_CREATE;

	int rc = sqlite3_open_v2(filepath.c_str(), &m_db, open_flags, NULL);
	if (rc != SQLITE_OK)
		throw std::runtime_error(STR_ERR_MSG_CANNOT_OPEN_DB);
}

Database::~Database()
{
	// TODO: Error check
	sqlite3_close_v2(m_db);
}

DBQuery Database::make_query(const std::string& sql_stmt_str)
{
	return DBQuery(m_db, sql_stmt_str);
}

std::vector<std::string> Database::get_table_names()
{
	std::vector<std::string> table_names;
	auto query = make_query("SELECT name FROM sqlite_master WHERE type='table'");
	query.execute([&](const DBRow& row) {
		table_names.push_back(row.get_col(0, DBRow::Column::Type::Text).value().get_text());
		return true;
	});
	return table_names;
}

Database::TableInfo Database::get_table_info(const std::string& table_name)
{
	TableInfo table_info;
	table_info.name = table_name;

	auto query = make_query("PRAGMA table_info(" + table_name + ")");
	query.execute([&](const DBRow& row)
		{
			auto col_name = row.get_col("name", DBRow::Column::Type::Text).value().get_text();
			auto col_type = row.get_col("type", DBRow::Column::Type::Text).value().get_text();

			table_info.columns[col_name] = string_to_column_type(col_type);

			return true;
		}
	);

	return table_info;
}

bool Database::has_instance()
{
	return get_instance() != nullptr;
}

std::shared_ptr<Database> Database::get_instance()
{
	return s_singleton;
}

void Database::set_instance(std::shared_ptr<Database> db)
{
	s_singleton = db;
}