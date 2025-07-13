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
    [ "\"\"" "\""! ]*

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

const std::string& CSVReader::get_cell(size_t row, size_t col) const
{
	if (row >= m_data.size() || col >= m_data[row].size())
		throw std::out_of_range("CSVReader: Index out of range");
	return m_data[row][col];
}