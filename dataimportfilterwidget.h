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
		std::vector<std::string> values;
	};
public:
	Filter(const std::string& filter_in_text, const std::string& filter_out_text, const qrawlr::Grammar& grammar);
public:
	bool is_valid() const;
	bool matches(const std::string& value) const;
	std::string apply(const std::string& value) const;
private:
	bool m_is_valid;
	FilterType m_type;
	std::vector<SubFilter> m_filter_in_a;
	std::vector<SubFilter> m_filter_in_b;
	std::vector<std::pair<bool, std::string>> m_filter_out;
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
private:
	static qrawlr::Grammar s_grammar;
};

