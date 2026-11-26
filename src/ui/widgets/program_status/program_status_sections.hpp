#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

namespace shekinah::ui {
namespace {

[[maybe_unused]] QWidget* make_section_header(const QString& title,
                                              QWidget* parent,
                                              QWidget* trailing_widget = nullptr,
                                              QLabel** title_out = nullptr) {
    auto* bar = new QWidget{parent};
    bar->setObjectName(QStringLiteral("drawer-section-header"));
    bar->setProperty("sectionHeader", true);
    bar->setFixedHeight(24);
    auto* row = new QHBoxLayout(bar);
    row->setContentsMargins(8, 0, 6, 0);
    row->setSpacing(4);
    auto* label = new QLabel{title, bar};
    label->setProperty("sectionTitle", true);
    row->addWidget(label);
    row->addStretch(1);
    if (trailing_widget != nullptr) {
        row->addWidget(trailing_widget);
    }
    if (title_out != nullptr) {
        *title_out = label;
    }
    return bar;
}

}  // namespace
}  // namespace shekinah::ui
