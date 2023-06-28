// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "historyview.h"

#include <QHeaderView>
#include <QStandardItemModel>

class HistoryViewPrivate
{
    friend class HistoryView;
    QStandardItemModel *model{nullptr};
    int historyColCount = 4;
};

HistoryView::HistoryView(QWidget *parent)
    : QTableView (parent)
    , d (new HistoryViewPrivate)
{
    d->model = new QStandardItemModel();
    setFocusPolicy(Qt::NoFocus);
    d->model->setColumnCount(4);
    setModel(d->model);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);//设置选中模式为选中行
    setSelectionMode(QAbstractItemView::SingleSelection);//设置选中单个
    setFrameShape(QFrame::NoFrame);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::Stretch);
    horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    horizontalHeader()->hide(); // 隐藏列头方法
    verticalHeader()->hide(); // 隐藏行号方法
}

HistoryData HistoryView::topData() const
{
    if (d->model->rowCount() > 0) {
        return HistoryView::createData(0);
    }
    return {};
}

void HistoryView::setDatas(const HistoryDatas &datas)
{
    setUpdatesEnabled(false);
    for (auto val : datas) {
        d->model->appendRow(createRow(val));
    }
    setUpdatesEnabled(true);
}

void HistoryView::insertTopDatas(const HistoryDatas &datas)
{
    setUpdatesEnabled(false);
    for (int i = 0; i < datas.size(); ++i) {
        d->model->insertRow(i, createRow(datas.at(i)));
    }
    setUpdatesEnabled(true);
}

void HistoryView::addData(const HistoryData &one)
{
    setUpdatesEnabled(false);
    d->model->appendRow(createRow(one));
    setUpdatesEnabled(true);
}

QString HistoryView::description(int row) const
{
    QString result = "";
    if (row >= 0 && row < d->model->rowCount()) {
        auto item = d->model->item(row);
        if (item) {
            result = item->data(HistoryRole::DescriptionRole).toString();
        }
    }
    return result;
}

RevisionFiles HistoryView::revisionFiles(int row) const
{
    RevisionFiles rFiles{};
    if (row >= 0 && row < d->model->rowCount()) {
        auto item = d->model->item(row);
        if (item) {
            auto variant = item->data(HistoryRole::RevisionFilesRole);
            if (variant.canConvert<RevisionFiles>()) {
                rFiles = qvariant_cast<RevisionFiles>(variant);
            }
        }
    }
    return rFiles;
}

HistoryData HistoryView::data(int row) const
{
    return createData(row);
}

QList<QStandardItem *> HistoryView::createRow(const HistoryData &data) const
{
    QList<QStandardItem *> result;
    if(!data.isValid())
        return result;

    auto revisionItem = new QStandardItem(data.revision);
    revisionItem->setData(data.description, HistoryRole::DescriptionRole);
    revisionItem->setData(QVariant::fromValue(data.changedFiles), HistoryRole::RevisionFilesRole);
    result << revisionItem;
    result << new QStandardItem(data.user);
    result << new QStandardItem(data.dateTime);
    result << new QStandardItem(data.lineCount);
    return result;
}

HistoryData HistoryView::createData(int row) const
{
    HistoryData result;

    if (row <=0 || row > d->model->rowCount())
        return result;

    QStandardItem *revisionItem = d->model->item(row, 0);
    QStandardItem *userItem = d->model->item(row, 1);
    QStandardItem *dateTimeItem = d->model->item(row, 2);
    QStandardItem *lineCountItem = d->model->item(row, 3);

    if (revisionItem) {
        result.revision = revisionItem->data(Qt::DisplayRole).toString();
        result.description = revisionItem->data(HistoryRole::DescriptionRole).toString();
        auto rFiles = revisionItem->data(HistoryRole::RevisionFilesRole);
        if (rFiles.canConvert<RevisionFiles>())
            result.changedFiles = qvariant_cast<RevisionFiles>(rFiles);
    }
    if (userItem)
        result.user = userItem->data(Qt::DisplayRole).toString();
    if (dateTimeItem)
        result.dateTime = dateTimeItem->data(Qt::DisplayRole).toString();
    if (lineCountItem)
        result.lineCount = lineCountItem->data(Qt::DisplayRole).toString();

    return result;
}
