#include "stdafx.h"
#include "CSVReader.h"

#include <libQrawlr.h>

const char* QRAWLR_CSV_GRAMMAR = R"(
CSV:
    Row+ Newline*_

Row:
	Cell (","_ Cell)* Newline_

Cell(hidden):
	"\""_ QuotedString "\""_
    Newline!~_ UnquotedString
	""

QuotedString(hidden fuse):
    [ "\"\""->"\"" "\""! ]*

UnquotedString(hidden fuse):
    [ "," Newline ]!+

Newline:
	[ "\n" "\r\n" ]
)";

CSVReader::CSVReader(const std::string& filepath)
{
	try
	{
		auto grammar = qrawlr::Grammar::load_from_text(QRAWLR_CSV_GRAMMAR, "QRAWLR_CSV_GRAMMAR");

		auto text = qrawlr::read_file(filepath);
		auto result = grammar.apply_to(text, "CSV", filepath);

		if (!result.tree || result.pos_end.index < text.size())
			throw qrawlr::GrammarException(
				"Parsing error at line " + std::to_string(result.pos_end.line) +
				" column " + std::to_string(result.pos_end.column) +
				" in file: " + filepath
			);

		auto node = qrawlr::expect_node(result.tree, "CSV", nullptr);
		
		auto& rows = node->get_children();

		m_data.resize(rows.size());

		size_t col_count;
		{
			auto first_row = qrawlr::expect_node(rows[0], "Row", nullptr);
			col_count = first_row->get_children().size();
		}

		size_t row_index = 0;
		for (auto& _row : rows)
		{
			auto row = qrawlr::expect_node(_row, "Row", nullptr);
			auto& cells = row->get_children();

			if (cells.size() != col_count)
				throw qrawlr::GrammarException(
					"Row has inconsistent number of columns in file: " + filepath
				);

			m_data[row_index].reserve(col_count);

			size_t col_index = 0;
			for (auto& _cell : cells)
			{
				auto cell = qrawlr::expect_leaf(_cell, nullptr);

				m_data[row_index].push_back(cell->get_value());

				++col_index;
			}

			++row_index;
		}
	}
	catch (const qrawlr::GrammarException& e)
	{
		throw std::runtime_error("CSVReader: " + std::string(e.what()));
	}
}

CSVReader::CSVReader(const std::vector<std::vector<std::string>>& data)
	: m_data(data)
{
}

size_t CSVReader::get_row_count() const
{
	return m_data.size();
}

size_t CSVReader::get_col_count() const
{
	if (m_data.empty())
		return 0;
	return m_data[0].size();
}

std::optional<std::string> CSVReader::get_cell(size_t row, size_t col) const
{
	if (row >= m_data.size() || col >= m_data[row].size())
		return {};

	return m_data[row][col];
}

bool CSVReader::set_cell(size_t row, size_t col, const std::string& content)
{
	if (row >= m_data.size() || col >= m_data[row].size())
		return false;

	m_data[row][col] = content;

	return true;
}

CSVReader CSVReader::make_sub_table(size_t row, size_t col, size_t row_count, size_t col_count, bool keep_header) const
{
	std::vector<std::vector<std::string>> sub_data;

	row_count = std::min(row_count, get_row_count() - row);
	col_count = std::min(col_count, get_col_count() - col);

	if (row_count <= 0 || col_count <= 0)
		return CSVReader();

	auto add_row = [&](size_t row_id)
		{
			auto row_it = m_data[row_id].begin() + col;
			sub_data.emplace_back(row_it, row_it + col_count);
		};

	if (keep_header && get_row_count() > 0)
		add_row(0);

	sub_data.resize(row_count, std::vector<std::string>(col_count, ""));
	sub_data.reserve(row_count + (keep_header ? 1 : 0));

	for (size_t row_id = row; row_id < row + row_count; ++row_id)
		add_row(row_id);

	return CSVReader(sub_data);
}