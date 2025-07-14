#include "stdafx.h"
#include "DBQuery.h"

DBQuery::DBQuery(sqlite3* db, const std::string& sql_stmt_str)
	: m_db(db), m_sql_stmt_str(sql_stmt_str)
{
	;
}

bool DBQuery::execute(Callback_Row cb_row, Callback_Table cb_table)
{
	m_err_msg.clear();

	int rc;
	const char* stmt_str = m_sql_stmt_str.c_str();

	while (stmt_str)
	{
		// Prepare statement
		sqlite3_stmt* stmt;
		const char* stmt_tail;

		rc = sqlite3_prepare_v2(
			m_db, stmt_str, -1,
			&stmt, &stmt_tail
		);

		if (!stmt)
		{
			if (rc)
			{
				m_err_msg = sqlite3_errmsg(m_db);
				return true;
			}
			else
			{
				return false;
			}
		}

		int col_cnt = sqlite3_column_count(stmt);
		// If callback for table is set, call it with column names
		if (cb_table)
		{
			std::vector<std::pair<std::string, DBRow::Column::Type>> columns;
			columns.reserve(col_cnt);

			for (int i = 0; i < col_cnt; ++i)
			{
				const char* col_name = sqlite3_column_name(stmt, i);
				auto col_type = conv_col_type(sqlite3_column_type(stmt, i));
				columns.push_back({ col_name ? col_name : "", col_type });
			}

			if (!cb_table(columns))
			{
				sqlite3_finalize(stmt);
				return false;
			}
		}

		// Step through the statement
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
		{
			auto row = DBRow(stmt);

			if (!cb_row)
				continue;

			// If the callback returns false, no more rows should be sampled
			if (!cb_row(row))
			{
				sqlite3_finalize(stmt);
				return false;
			}
		}

		// Check for errors
		if (rc != SQLITE_DONE && rc != SQLITE_ROW)
			m_err_msg = sqlite3_errmsg(m_db);

		// Cleanup statement
		sqlite3_finalize(stmt);

		// Skip remaining statements if error occurs
		if (!m_err_msg.empty())
			break;

		stmt_str = stmt_tail;
	}

	return stmt_str[0] != '\0';
}

const std::string& DBQuery::get_err_msg() const
{
	return m_err_msg;
}