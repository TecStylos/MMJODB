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
		( ":"_ Whitespace?_ Identifier Whitespace?_ )?
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
		( ":"_  Whitespace?_Identifier Whitespace?_ )?
		"]"_
	)

MatchPatternPattern:
	[
		String
		"d"
		"a"
        "."
	]+

Identifier(fuse):
    AlphaChar AlnumChar*

String(fuse):
    "\""_ StringChar+ "\""_

StringChar(hidden):
    EscapeSequence
	"\""!

EscapeSequence(fuse):
    (
        "\\"_
        [
            "0" "a" "b" "e" "f" "n" "r" "t" "v" "\\" "'" "\""
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

qrawlr::Grammar DataImportFilterWidget::s_grammar = qrawlr::Grammar::load_from_text(g_grammar_text, "<INTERNAL_FILTER_GRAMMAR_TEXT>");

Filter::Filter(const std::string& filter_in_text, const std::string& filter_out_text, const qrawlr::Grammar& grammar)
{
	;
}

bool Filter::is_valid() const
{
	// Implement validation logic for the filter
	return false; // Placeholder
}

bool Filter::matches(const std::string& value) const
{
	if (!is_valid())
		return false;
	// Implement matching logic based on the filter
	return false; // Placeholder
}

std::string Filter::apply(const std::string& value) const
{
	if (!is_valid())
		return value;

	// Implement logic to apply the filter to the value
	return value; // Placeholder
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
	return Filter(filter_in_text, filter_out_text, s_grammar);
}

void DataImportFilterWidget::on_buttonDelete_clicked()
{
	deleteLater();
}