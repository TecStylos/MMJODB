#pragma once

#include <functional>
#include "vendor/sqlite/sqlite3.h"

#include "DBRow.h"

class DBQuery
{
public:
	typedef std::function<bool (const DBRow&)> Callback_Row;
	typedef std::function<bool (const std::vector<std::pair<std::string, DBRow::Column::Type>>&)> Callback_Table;
public:
	DBQuery(sqlite3* db, const std::string& sql_stmt_str);
public:
	bool execute(Callback_Row cb_row, Callback_Table cb_table = nullptr);
	const std::string& get_err_msg() const;
private:
	sqlite3* m_db;
	std::string m_sql_stmt_str;
	std::string m_err_msg;
	Callback_Row m_temp_cb;
};