// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reportpane.h"
#include "codeporting.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QDebug>
#include <QHBoxLayout>

ReportPane::ReportPane(CodePorting *_codePorting, QWidget *parent) : QWidget(parent)
  , srcTableWidget(new QTableWidget(this))
  , libTableWidget(new QTableWidget(this))
  , codePorting(_codePorting)
{
    initTableWidget();
}

void ReportPane::refreshDispaly()
{
    auto &&srcReport = codePorting->getSourceReport();
    auto &&libReport = codePorting->getDependLibReport();
    refreshTableView(srcTableWidget, srcReport);
    refreshTableView(libTableWidget, libReport);
}

void ReportPane::srcCellSelected(int row, int col)
{
    qDebug() << "srcCellSelected: " << row << col;

    auto &&report = codePorting->getSourceReport();
    if (report.size()) {
        auto items = report[row];
        if (items.size() == CodePorting::kItemsCount) {
            QString range = items[CodePorting::kCodeRange];
            QRegularExpression reg("(?<=\\()(\\d)*, (\\d)*(?=\\))");
            auto match = reg.match(range);
            int startLine = 0;
            int endLine = 0;
            if (match.hasMatch()) {
                QStringList lines = match.captured().split(",");
                if (lines.size() == 2) {
                    startLine = lines.front().toInt();
                    endLine = lines.back().toInt();
                }
            }
            emit selectedChanged(items[CodePorting::kFilePath], items[CodePorting::kSuggestion], startLine, endLine);
        }
    }
}

void ReportPane::libCellSelected(int row, int col)
{
    qDebug() << "libCellSelected: " << row << col;
    // TODO(mozart)
}

void ReportPane::initTableWidget()
{
    setTableWidgetStyle(srcTableWidget, codePorting->getSrcItemNames());
    setTableWidgetStyle(libTableWidget, codePorting->getLibItemNames());

    connect(srcTableWidget, &QTableWidget::cellDoubleClicked, this, &ReportPane::srcCellSelected);
    connect(libTableWidget, &QTableWidget::cellDoubleClicked, this, &ReportPane::libCellSelected);

    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->addTab(srcTableWidget, tr("Source files to migrate"));

    tabWidget->addTab(libTableWidget, tr("Architecture-dependent library files"));
    tabWidget->setTabPosition(QTabWidget::South);

    auto hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 9);
    this->setLayout(hLayout);
    hLayout->addWidget(tabWidget);
}

void ReportPane::refreshTableView(QTableWidget *widget, const QList<QStringList> &report)
{
    if (widget && report.size() > 0) {
        widget->clearContents();

        int itemsCount = report.size();
        widget->setRowCount(itemsCount);

        int row = 0;
        int col = 0;
        for (auto itItem = report.begin(); itItem != report.end(); ++itItem) {
            for (auto data : *itItem) {
                widget->setItem(row, col, new QTableWidgetItem(data));
                col++;
            }
            row++;
            col = 0;
        }
        widget->resizeColumnsToContents();
    }
}

void ReportPane::setTableWidgetStyle(QTableWidget *tableWidget, const QStringList &colNames)
{
    tableWidget->setColumnCount(colNames.count());
    tableWidget->setHorizontalHeaderLabels(colNames);
    tableWidget->verticalHeader()->setVisible(true);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setShowGrid(true);
}
