#pragma once

#include <map>
#include <vector>
#include <optional>
#include <variant>

#include "vendor/sqlite/sqlite3.h"

class DBRow
{
public:
	class Column
	{
	public:
		enum class Type
		{
			Null,
			Int64,
			Float64,
			Text,
			Blob,
			None
		};
	private:
		Type m_type;
		std::variant<int64_t, double, std::string, std::vector<char>> m_data;
	public:
		Column();
		Column(Type type);
		Column(int64_t data);
		Column(double data);
		Column(const std::string& data);
		Column(const std::vector<char>& data);
		Column(const char* data, int size);
	public:
		Type get_type() const;
		int64_t get_int64() const;
		double get_float64() const;
		const std::string& get_text() const;
		const std::vector<char>& get_blob() const;
	};
public:
	DBRow(sqlite3_stmt* stmt);
public:
	int get_col_cnt() const;
	std::optional<std::string> get_col_name(int index) const;
	std::optional<Column::Type> get_col_type(int index) const;
	std::optional<Column::Type> get_col_type(const std::string& name) const;
	std::optional<Column> get_col(int index, Column::Type type = Column::Type::None) const;
	std::optional<Column> get_col(const std::string& name, Column::Type type = Column::Type::None) const;
public:
	int get_name_index(const std::string& name) const;
private:
	bool index_out_of_bounds(int index) const;
private:
	static Column::Type conv_col_type(int type);
	static int conv_col_type(Column::Type type);
private:
	sqlite3_stmt* m_stmt;
	int m_col_cnt;
	mutable std::map<std::string, int> m_name_mapping;
};