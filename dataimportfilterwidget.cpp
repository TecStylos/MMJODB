#include "stdafx.h"
#include "dataimportfilterwidget.h"

#include "Strings.h"

const char* g_grammar_text = R"(
FilterOutput:
    (
        Whitespace?_
        (
            [
                MatchInsertion
                String
            ]
            Whitespace?_
        )+
    )

MatchInsertion:
    (
        "["_ Whitespace?_
		Identifier Whitespace?_
		"]"_
	)

FilterInput:
	(
		Whitespace?_
		[
			( Matcher MatchAny Matcher )
			( MatchAny? Matcher MatchAny? )
		]
	)

MatchAny:
	(
		"["_ Whitespace?_ "%"_ Whitespace?_
		[
			( ":"_ Whitespace?_ Identifier Whitespace?_ )
			EmptyIdentifier
		]
		"]"_
	)

Matcher:
    (
		[ String MatchPattern ]
		Whitespace?_
	)+

MatchPattern:
    (
		"["_ Whitespace?_ MatchPatternPattern Whitespace?_
		[
			( ":"_  Whitespace?_ Identifier Whitespace?_ )
			EmptyIdentifier
		]
		"]"_
	)

MatchPatternPattern:
	(
		[
			String
			"d"
			"a"
			"."
		]
		Whitespace?_
	)+

Identifier(fuse):
    AlphaChar AlnumChar*

EmptyIdentifier:
	""

String(fuse):
    "\""_ StringChar+ "\""_

StringChar(hidden):
    EscapeSequence
	"\""!

EscapeSequence(collapse):
    (
        "\\"_
        [
            "0"->"\0"
			"a"->"\a"
			"b"->"\b"
			"e"->"\e"
			"f"->"\f"
			"n"->"\n"
			"r"->"\r"
			"t"->"\t"
			"v"->"\v"
			"\\"->"\\"
			"'"->"\'"
			"\""->"\""
            (
                "x" (HexChar HexChar)
            )
        ]
    )

AlphaChar(hidden): [ 'az' 'AZ' ]
AlnumChar(hidden): [ AlphaChar DecChar ]
DecChar(hidden): '09'
HexChar(hidden): [ '09' 'af' 'AF' ]
Whitespace(fuse): [ " " "\t" ]+
)";

const char* g_grammar_file_name = "<INTERNAL_FILTER_GRAMMAR_TEXT>";

qrawlr::Grammar Filter::s_grammar = qrawlr::Grammar::load_from_text(g_grammar_text, g_grammar_file_name);

Filter::Filter(const std::string& filter_in_text, const std::string& filter_out_text)
{
	m_is_valid = true; // Assume valid until proven otherwise

	try
	{
		create_input_filter(filter_in_text);
		create_output_filter(filter_out_text);
	}
	catch (const qrawlr::GrammarException& e)
	{
		m_is_valid = false;
		return;
	}

	if (m_is_valid && !check_filter_compatibility())
		m_is_valid = false;
}

bool Filter::is_valid() const
{
	return m_is_valid;
}

bool Filter::matches(const std::string& value) const
{
	if (!is_valid())
		return false;
	
	return apply(value).has_value();
}

std::optional<std::string> Filter::apply(const std::string& value) const
{
	if (!is_valid())
		return {};

	std::map<std::string, std::string> named_matches;

	auto match_a = find_match(value, m_filter_in_a, false);
	auto match_b = find_match(value, m_filter_in_b, true);

	named_matches.merge(match_a.named_matches);
	named_matches.merge(match_b.named_matches);

	// Check if match_a is correctly detected
	if (!m_filter_in_a.empty())
	{
		if (
			(match_a.beg < 0) ||
			(match_a.beg != 0 && m_type != FilterType::Contains) ||
			(match_a.end != value.size() && m_type == FilterType::Exact)
			)
			return {};
	}

	// Check if match_b is correctly detected
	if (!m_filter_in_b.empty())
		if (match_b.end != value.size())
			return {};

	// Check if match_a and match_b collide
	if (!m_filter_in_a.empty() && !m_filter_in_b.empty())
	{
		if (match_a.end > match_b.beg)
			return {};
	}

	auto handle_any_name = [&](int begin, int end, const std::string& name)
		{
			if (name.empty())
				return;

			named_matches[name] = value.substr(begin, end - begin);
		};

	// get values for MatchAny sub filters
	switch (m_type)
	{
	case FilterType::Exact:
		break;
	case FilterType::Contains:
		handle_any_name(0, match_a.beg, m_filter_in_match_any_name_1);
		handle_any_name(match_a.end, value.size(), m_filter_in_match_any_name_2);
		break;
	case FilterType::BeginsWith:
		handle_any_name(match_a.end, value.size(), m_filter_in_match_any_name_1);
		break;
	case FilterType::EndsWith:
		handle_any_name(0, match_b.beg, m_filter_in_match_any_name_1);
		break;
	case FilterType::BeginsAndEndsWith:
		handle_any_name(match_a.end, match_b.beg, m_filter_in_match_any_name_1);
		break;
	}


	// Generate output
	std::string out;
	for (auto& [f_is_str, f_val] : m_filter_out)
	{
		if (f_is_str)
		{
			out.append(f_val);
			continue;
		}

		out.append(named_matches[f_val]);
	}

	return out;
}

void Filter::create_input_filter(const std::string& filter_in_text)
{
	// Parse the input filter text and populate m_filter_in_a and m_filter_in_b
	// This is a placeholder implementation
	m_filter_in_a.clear();
	m_filter_in_b.clear();
	m_filter_in_match_any_name_1.clear();
	m_filter_in_match_any_name_2.clear();

	auto r = s_grammar.apply_to(filter_in_text, "FilterInput", g_grammar_file_name);

	if (!r.tree || r.pos_end.index != filter_in_text.size())
	{
		m_is_valid = false;
		return;
	}

	auto root_node = qrawlr::expect_node(r.tree, "FilterInput", nullptr);

	{
		auto& children = root_node->get_children();
		switch (children.size())
		{
		case 1:
			m_type = FilterType::Exact;
			break;
		case 2:
			if (qrawlr::is_node(children[0], "Matcher"))
				m_type = FilterType::BeginsWith;
			else
				m_type = Filter::FilterType::EndsWith;
			break;
		case 3:
			if (qrawlr::is_node(children[0], "Matcher"))
				m_type = FilterType::BeginsAndEndsWith;
			else
				m_type = FilterType::Contains;
			break;
		default:
			m_is_valid = false;
			return;
		}
	}

	switch (m_type)
	{
	case FilterType::Exact:
		m_filter_in_a = parse_matcher(qrawlr::expect_child_node(root_node, "0", nullptr));
		break;
	case FilterType::BeginsWith:
		m_filter_in_a = parse_matcher(qrawlr::expect_child_node(root_node, "0", nullptr));
		m_filter_in_match_any_name_1 = qrawlr::expect_child_leaf(root_node, "1.0.0", nullptr)->get_value();
		break;
	case FilterType::EndsWith:
		m_filter_in_match_any_name_1 = qrawlr::expect_child_leaf(root_node, "0.0.0", nullptr)->get_value();
		m_filter_in_b = parse_matcher(qrawlr::expect_child_node(root_node, "1", nullptr));
		break;
	case FilterType::BeginsAndEndsWith:
		m_filter_in_a = parse_matcher(qrawlr::expect_child_node(root_node, "0", nullptr));
		m_filter_in_match_any_name_1 = qrawlr::expect_child_leaf(root_node, "1.0.0", nullptr)->get_value();
		m_filter_in_b = parse_matcher(qrawlr::expect_child_node(root_node, "2", nullptr));
		break;
	case FilterType::Contains:
		m_filter_in_match_any_name_1 = qrawlr::expect_child_leaf(root_node, "0.0.0", nullptr)->get_value();
		m_filter_in_a = parse_matcher(qrawlr::expect_child_node(root_node, "1", nullptr));
		m_filter_in_match_any_name_2 = qrawlr::expect_child_leaf(root_node, "2.0.0", nullptr)->get_value();
		break;
	}
}

std::vector<Filter::SubFilter> Filter::parse_matcher(qrawlr::ParseTreeNodeRef node)
{
	std::vector<SubFilter> subfilters;

	for (auto& child : node->get_children())
	{
		SubFilter sf;

		auto filter = qrawlr::expect_node(child, nullptr);
		if (filter->get_name() == "String")
		{
			sf.name = "";
			sf.values.emplace_back(true, qrawlr::expect_child_leaf(filter, "0", nullptr)->get_value());
		}
		else if (filter->get_name() == "MatchPattern")
		{
			sf.name = qrawlr::expect_child_leaf(filter, "1.0", nullptr)->get_value();
			
			for (auto& pattern : qrawlr::expect_child_node(filter, "0", nullptr)->get_children())
			{
				if (qrawlr::is_leaf(pattern))
					sf.values.emplace_back(false, qrawlr::expect_leaf(pattern, nullptr)->get_value());
				else
					sf.values.emplace_back(true, qrawlr::expect_child_leaf(pattern, "0", nullptr)->get_value());
			}
		}
		else
		{
			throw std::runtime_error("This should never occur!");
		}

		subfilters.push_back(sf);
	}

	return subfilters;
}

bool Filter::check_filter_compatibility() const
{
	std::set<std::string> names_in;
	
	if (!m_filter_in_match_any_name_1.empty()) names_in.insert(m_filter_in_match_any_name_1);
	if (!m_filter_in_match_any_name_2.empty()) names_in.insert(m_filter_in_match_any_name_2);

	auto helper = [&](const std::vector<SubFilter> filter_in)
		{
			for (auto& sub : filter_in)
			{
				if (sub.name.empty())
					continue;

				if (names_in.find(sub.name) != names_in.end())
					throw std::runtime_error("Duplicate name");

				names_in.insert(sub.name);
			}
		};

	helper(m_filter_in_a);
	helper(m_filter_in_b);

	for (auto& p : m_filter_out)
	{
		if (p.first) // is string and not a match insertion
			continue;

		if (names_in.find(p.second) == names_in.end())
			return false;
	}

	return true;
}

Filter::Match Filter::find_match(const std::string& value, const std::vector<SubFilter>& filter, bool reverse) const
{
	int beg, end, chg;
	if (!reverse)
	{
		beg = 0;
		end = value.size();
		chg = 1;
	}
	else
	{
		beg = value.size() - 1;
		end = -1;
		chg = -1;
	}

	auto get_match_at = [&](int pos)
		{
			Match m;
			m.beg = pos;

			for (auto& sf : filter)
			{
				int sf_beg = pos;

				for (auto& [v_is_str, v_value] : sf.values)
				{
					if (pos >= value.size())
						return Match();

					if (v_is_str)
					{
						if (value.find(v_value, pos) == pos)
							pos += v_value.size();
						else
							return Match();
					}
					else
					{
						if (v_value == "d")
						{
							if (!std::isdigit(value[pos]))
								return Match();
						}
						else if (v_value == "a")
						{
							if (!std::isalpha(value[pos]))
								return Match();
						}
						else if (v_value == ".")
						{
							; // pass, match every character
						}
						else
						{
							throw std::runtime_error("This should never occur!");
						}

						++pos;
					}
				}

				if (!sf.name.empty())
					m.named_matches.emplace(sf.name, value.substr(sf_beg, pos - sf_beg));
			}

			m.end = pos;

			return m;
		};

	for (int cur = beg; cur != end; cur += chg)
	{
		Match m = get_match_at(cur);
		if (m.beg >= 0)
			return m;
	}

	return {};
}

void Filter::create_output_filter(const std::string& filter_out_text)
{
	m_filter_out.clear();

	auto r = s_grammar.apply_to(filter_out_text, "FilterOutput", g_grammar_file_name);

	if (!r.tree || r.pos_end.index != filter_out_text.size())
	{
		m_is_valid = false;
		return;
	}

	auto root_node = qrawlr::expect_node(r.tree, "FilterOutput", nullptr);
	for (auto& child : root_node->get_children())
	{
		auto filter_part = qrawlr::expect_node(child, nullptr);
		if (filter_part->get_name() == "MatchInsertion")
		{
			auto& name = qrawlr::expect_child_leaf(filter_part, "Identifier.0", nullptr)->get_value();
			m_filter_out.emplace_back(false, name);
		}
		else if (filter_part->get_name() == "String")
		{
			auto& text = qrawlr::expect_child_leaf(filter_part, "0", nullptr)->get_value();
			m_filter_out.emplace_back(true, text);
		}
		else
		{
			m_is_valid = false;
			return;
		}
	}
}

DataImportFilterWidget::DataImportFilterWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

DataImportFilterWidget::~DataImportFilterWidget()
{}

int DataImportFilterWidget::test_filter(const CSVReader& csv, int col_id)
{
	int matches = 0;
	int row_count = (int)csv.get_row_count();

	auto filter = load_filter();

	if (!filter.is_valid())
	{
		ui.labelFilterResult->setText(STR_ERR_LBL_INVALID_FILTER);
		return 0;
	}

	for (int i = 1; i < row_count; ++i) // Start from 1 to skip header row
		if (filter.matches(csv.get_cell(i, col_id)))
			++matches;

	ui.labelFilterResult->setText("Treffer: " + QString::number(matches) + "/" + QString::number(row_count - 1));

	return matches;
}

Filter DataImportFilterWidget::load_filter()
{
	std::string filter_in_text = ui.lineEditInputFormat->text().toStdString();
	std::string filter_out_text = ui.lineEditOutputFormat->text().toStdString();
	return Filter(filter_in_text, filter_out_text);
}

void DataImportFilterWidget::on_buttonDelete_clicked()
{
	deleteLater();
}