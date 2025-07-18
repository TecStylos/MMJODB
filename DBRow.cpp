#include "stdafx.h"
#include "DBRow.h"

#include "Strings.h"

DBRow::Column::Column()
	: m_type(Type::Null)
{
	m_data = int64_t(0);
}

DBRow::Column::Column(Type type)
	: m_type(type)
{
	switch (type)
	{
	case Type::Null:
		m_data = int64_t(0);
		break;
	case Type::Int64:
		m_data = int64_t(0);
		break;
	case Type::Float64:
		m_data = double(0.0);
		break;
	case Type::Text:
		m_data = std::string("");
		break;
	case Type::Blob:
		m_data = std::vector<char>();
		break;
	case Type::None:
		break;
	default:
		throw std::runtime_error(STR_INT_ERR_UNKNOWN_COLUMN_TYPE);
	}
}

DBRow::Column::Column(int64_t data)
	: m_type(Type::Int64), m_data(data)
{
	;
}

DBRow::Column::Column(double data)
	: m_type(Type::Float64), m_data(data)
{
	;
}

DBRow::Column::Column(const std::string& data)
	: m_type(Type::Text), m_data(data)
{
	;
}

DBRow::Column::Column(const std::vector<char>& data)
	: m_type(Type::Blob), m_data(data)
{
	;
}

DBRow::Column::Column(const char* data, int size)
	: Column(std::vector<char>(data, data + size))
{
	;
}

DBRow::Column::Type DBRow::Column::get_type() const
{
	return m_type;
}

int64_t DBRow::Column::get_int64() const
{
	if (m_type != Type::Int64)
		throw std::runtime_error(STR_INT_ERR_COL_TYPE_MISMATCH_EXP_INT64);
	return std::get<int64_t>(m_data);
}

double DBRow::Column::get_float64() const
{
	if (m_type != Type::Float64)
		throw std::runtime_error(STR_INT_ERR_COL_TYPE_MISMATCH_EXP_FLOAT64);
	return std::get<double>(m_data);
}

const std::string& DBRow::Column::get_text() const
{
	if (m_type != Type::Text)
		throw std::runtime_error(STR_INT_ERR_COL_TYPE_MISMATCH_EXP_TEXT);
	return std::get<std::string>(m_data);
}

const std::vector<char>& DBRow::Column::get_blob() const
{
	if (m_type != Type::Blob)
		throw std::runtime_error(STR_INT_ERR_COL_TYPE_MISMATCH_EXP_BLOB);
	return std::get<std::vector<char>>(m_data);
}

DBRow::DBRow(sqlite3_stmt* stmt)
	: m_stmt(stmt), m_col_cnt(sqlite3_column_count(stmt))
{
	;
}

int DBRow::get_col_cnt() const
{
	return m_col_cnt;
}

std::optional<std::string> DBRow::get_col_name(int index) const
{
	if (index_out_of_bounds(index))
		return {};

	std::string name = sqlite3_column_name(m_stmt, index);
	return name;
}

std::optional<DBRow::Column::Type> DBRow::get_col_type(int index) const
{
	if (index_out_of_bounds(index))
		return {};

	return conv_col_type(sqlite3_column_type(m_stmt, index));
}

std::optional<DBRow::Column::Type> DBRow::get_col_type(const std::string& name) const
{
	return get_col_type(get_name_index(name));
}

std::optional<DBRow::Column> DBRow::get_col(int index, Column::Type type) const
{
	if (index_out_of_bounds(index))
		return {};

	if (type == Column::Type::None)
		type = get_col_type(index).value();
	int sql_type = conv_col_type(type);

	Column col;

	switch (type)
	{
	case Column::Type::Null:
	{
		col = Column(Column::Type::Null);
		break;
	}
	case Column::Type::Int64:
	{
		col = Column(sqlite3_column_int64(m_stmt, index));
		break;
	}
	case Column::Type::Float64:
	{
		col = Column(sqlite3_column_double(m_stmt, index));
		break;
	}
	case Column::Type::Text:
	{
		const char* data = (const char*)sqlite3_column_text(m_stmt, index);
		if (!data)
			col = Column(Column::Type::Null);
		else
			col = Column(std::string(data, data + strlen(data)));
		break;
	}
	case Column::Type::Blob:
	{
		const char* data = (const char*)sqlite3_column_blob(m_stmt, index);
		if (!data)
			col = Column(Column::Type::Null);
		else
			col = Column(data, sqlite3_column_bytes(m_stmt, index));
		break;
	}
	case Column::Type::None:
		throw std::runtime_error("This should never occur!");
	default:
		throw std::runtime_error(STR_INT_ERR_UNKNOWN_COLUMN_TYPE);
	}

	return col;
}

std::optional<DBRow::Column> DBRow::get_col(const std::string& name, Column::Type type) const
{
	return get_col(get_name_index(name), type);
}

int DBRow::get_name_index(const std::string& name) const
{
	if (m_name_mapping.empty())
	{
		// Generate name mapping
		for (int i = 0; i < get_col_cnt(); ++i)
		{
			std::string name = sqlite3_column_name(m_stmt, i);
			m_name_mapping.insert({ name, i });
		}
	}

	auto it = m_name_mapping.find(name);
	if (it == m_name_mapping.end())
		return -1;

	return it->second;
}

bool DBRow::index_out_of_bounds(int index) const
{
	return index < 0 || index >= get_col_cnt();
}

DBRow::Column::Type conv_col_type(int type)
{
	switch (type)
	{
	case SQLITE_INTEGER:
		return DBRow::Column::Type::Int64;
	case SQLITE_FLOAT:
		return DBRow::Column::Type::Float64;
	case SQLITE_BLOB:
		return DBRow::Column::Type::Blob;
	case SQLITE_NULL:
		return DBRow::Column::Type::Null;
	case SQLITE_TEXT:
		return DBRow::Column::Type::Text;
	default:
		return DBRow::Column::Type::None;
	}
}

int conv_col_type(DBRow::Column::Type type)
{
	switch (type)
	{
	case DBRow::Column::Type::Null:
		return SQLITE_NULL;
	case DBRow::Column::Type::Int64:
		return SQLITE_INTEGER;
	case DBRow::Column::Type::Float64:
		return SQLITE_FLOAT;
	case DBRow::Column::Type::Text:
		return SQLITE_TEXT;
	case DBRow::Column::Type::Blob:
		return SQLITE_BLOB;
	case DBRow::Column::Type::None:
		return -1;
	default:
		return -1;
	}
}

std::string column_type_to_string(DBRow::Column::Type type)
{
	switch (type)
	{
	case DBRow::Column::Type::Null:
		return "NULL";
	case DBRow::Column::Type::Int64:
		return "INT";
	case DBRow::Column::Type::Float64:
		return "REAL";
	case DBRow::Column::Type::Text:
		return "TEXT";
	case DBRow::Column::Type::Blob:
		return "BLOB";
	default:
		return "UNKNOWN";
	}
}

DBRow::Column::Type string_to_column_type(const std::string& type_str)
{
	if (type_str == "INT" || type_str == "INTEGER")
		return DBRow::Column::Type::Int64;
	else if (type_str == "REAL")
		return DBRow::Column::Type::Float64;
	else if (type_str == "TEXT")
		return DBRow::Column::Type::Text;
	else if (type_str == "BLOB")
		return DBRow::Column::Type::Blob;
	else if (type_str == "NULL")
		return DBRow::Column::Type::Null;
	else
		return DBRow::Column::Type::None; // Unknown type
}