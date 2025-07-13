#pragma once

#include <string>

class CSVReader
{
public:
	CSVReader() = default;
	CSVReader(const std::string& filepath);
public:
	size_t get_row_count() const;
	size_t get_col_count() const;
	const std::string& get_cell(size_t row, size_t col) const;
private:
	std::vector<std::vector<std::string>> m_data;
};