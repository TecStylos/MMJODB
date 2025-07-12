#include "stdafx.h"
#include "Database.h"

std::shared_ptr<Database> Database::s_singleton = nullptr;

Database::Database(const std::string& filepath, bool allow_create)
{
	int open_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX;
	if (allow_create)
		open_flags |= SQLITE_OPEN_CREATE;

	int rc = sqlite3_open_v2(filepath.c_str(), &m_db, open_flags, NULL);
	if (rc != SQLITE_OK)
		throw std::runtime_error("Datenbank konnte nicht geoeffnet werden.");
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