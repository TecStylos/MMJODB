#pragma once

#include <QWidget>
#include "ui_dataimportfilterwidget.h"

#include <string>
#include <vector>
#include <libQrawlr.h>
#include "CSVReader.h"

class Filter
{
private:
	enum class FilterType
	{
		Exact, Contains,
		BeginsWith, EndsWith,
		BeginsAndEndsWith
	};
	struct SubFilter
	{
		std::string name;
		std::vector<std::pair<bool, std::string>> values; // bool -> is_string, string -> value/type
	};
public:
	Filter(const std::string& filter_in_text, const std::string& filter_out_text);
public:
	bool is_valid() const;
	bool matches(const std::string& value) const;
	std::optional<std::string> apply(const std::string& value) const;
private:
	void create_input_filter(const std::string& filter_in_text);
	void create_output_filter(const std::string& filter_out_text);
	std::vector<SubFilter> parse_matcher(qrawlr::ParseTreeNodeRef node);
	bool check_filter_compatibility() const;
private:
	bool m_is_valid;
	FilterType m_type;
	std::vector<SubFilter> m_filter_in_a;
	std::vector<SubFilter> m_filter_in_b;
	std::string m_filter_in_match_any_name_1;
	std::string m_filter_in_match_any_name_2;
	std::vector<std::pair<bool, std::string>> m_filter_out; // bool -> is_string, string -> value/name
private:
	static qrawlr::Grammar s_grammar;
};

class DataImportFilterWidget : public QWidget
{
	Q_OBJECT

public:
	DataImportFilterWidget(QWidget *parent = nullptr);
	~DataImportFilterWidget();
public:
	int test_filter(const CSVReader& csv, int col_id);
private:
	Filter load_filter();
private slots:
	void on_buttonDelete_clicked();
private:
	Ui::DataImportFilterWidgetClass ui;
};

