#pragma once

#include <QWidget>
#include <string>

std::string single_file_dialog(QWidget* parent, const std::string& title, const std::string& name_filter, bool for_read_else_write);