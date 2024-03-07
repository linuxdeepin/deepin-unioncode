// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reportpane.h"
#include "codeporting.h"
#include "codeportingmanager.h"

#include <DTabWidget>
#include <DTableWidget>
#include <DPushButton>
#include <DStackedWidget>
#include <DStyle>
#include <DGroupBox>
#include <DTreeView>
#include <DTableView>
#include <DStandardItem>
#include <DGuiApplicationHelper>

#include <QStandardItemModel>
#include <QHeaderView>
#include <QDebug>
#include <QHBoxLayout>

ReportPane::ReportPane(CodePorting *_codePorting, QWidget *parent)
    : DWidget(parent)
    , codePorting(_codePorting)
{
    initUI();
}

void ReportPane::refreshDispaly()
{
    auto &&srcReport = codePorting->getSourceReport();
    auto &&libReport = codePorting->getDependLibReport();

    setViewItem(srcView, srcReport);
    setViewItem(libView, libReport);
}

void ReportPane::srcCellSelected(const QModelIndex &index)
{
    auto &&report = codePorting->getSourceReport();
    if (report.size()) {
        auto items = report[index.row()];
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

void ReportPane::onChangeReportList(const QString &listName)
{
    if (!srcView || !libView)
        return;

    srcView->hide();
    libView->hide();

    if (listName == REPORT_SRCLIST)
        srcView->show();
    else if (listName == REPORT_LIBLIST)
        libView->show();
}

void ReportPane::initUI()
{
    srcView = new DTreeView(this);
    srcView->setHeaderHidden(false);
    srcView->setEditTriggers(QListView::NoEditTriggers);
    srcView->setTextElideMode(Qt::ElideRight);

    libView = new DTreeView(this);
    libView->setHeaderHidden(false);
    libView->setEditTriggers(QListView::NoEditTriggers);
    libView->setTextElideMode(Qt::ElideRight);
    libView->hide();

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(srcView);
    vLayout->addWidget(libView);
}

void ReportPane::setViewItem(DTreeView *view, const QList<QStringList> &report)
{
    QStandardItemModel *model = new QStandardItemModel(view);
    view->setModel(model);

    for (auto row : report) {
        QList<QStandardItem*> items;
        for (auto col : row) {
            QStandardItem *item = new QStandardItem(col);
            item->setSizeHint(QSize(item->sizeHint().width(), 24));
            items << item;
        }
        model->appendRow(items);
    }

    QStringList colNames = codePorting->getSrcItemNames();
    for (int i = 0; i < colNames.count(); ++i) {
        model->setHeaderData(i, Qt::Horizontal, colNames[i], Qt::DisplayRole);
    }

    view->setAlternatingRowColors(true);

    connect(view, &QListView::doubleClicked, this, &ReportPane::srcCellSelected, Qt::UniqueConnection);
}
