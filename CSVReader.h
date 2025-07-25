#pragma once

#include <string>

class CSVReader
{
public:
	CSVReader() = default;
	CSVReader(const std::string& filepath);
private:
	CSVReader(const std::vector<std::vector<std::string>>& data);
public:
	size_t get_row_count() const;
	size_t get_col_count() const;
	std::optional<std::string> get_cell(size_t row, size_t col) const;
	bool set_cell(size_t row, size_t col, const std::string& content);
	CSVReader make_sub_table(size_t row, size_t col, size_t row_count, size_t col_count, bool keep_header = false) const;
private:
	std::vector<std::vector<std::string>> m_data;
};