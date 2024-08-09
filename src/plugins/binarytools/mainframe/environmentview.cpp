// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environmentview.h"
#include "binarytoolsconfigview.h"

#include "common/util/namevaluemodel.h"
#include "base/baseitemdelegate.h"

#include <DTableView>
#include <DCheckBox>
#include <DHeaderView>
#include <DFrame>
#include <DPushButton>
#include <DIconButton>

#include <QVBoxLayout>
#include <QHeaderView>
#include <QProcessEnvironment>

DWIDGET_USE_NAMESPACE

class EnvironmentViewPrivate
{
    friend class EnvironmentView;

    NameValueModel model;
    QVBoxLayout *vLayout = nullptr;
    DTableView *tableView = nullptr;
    BinaryToolsConfigView *configView = nullptr;
    DIconButton *appendButton = nullptr;
    DIconButton *deleteButton = nullptr;
    DIconButton *resetButton = nullptr;
};

EnvironmentView::EnvironmentView(DWidget *parent)
    : DWidget(parent), d(new EnvironmentViewPrivate)
{
    setAutoFillBackground(true);

    if (!d->vLayout)
        d->vLayout = new QVBoxLayout();
    setLayout(d->vLayout);

    if (!d->tableView) {
        d->tableView = new DTableView();
        d->tableView->setShowGrid(false);
        DHeaderView *headerView = d->tableView->horizontalHeader();
        headerView->setSectionResizeMode(DHeaderView::ResizeToContents);
        headerView->setDefaultAlignment(Qt::AlignLeft);
        d->tableView->verticalHeader()->hide();
        d->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        d->tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
        d->tableView->setTabletTracking(true);
        d->tableView->setLineWidth(0);
        d->tableView->setFrameShape(DFrame::NoFrame);
        d->tableView->setAlternatingRowColors(true);
    }
    d->vLayout->addWidget(d->tableView);
    d->tableView->setModel(&d->model);
    d->tableView->setItemDelegate(new BaseItemDelegate(d->tableView));

    //append
    d->appendButton = new DIconButton(this);
    d->appendButton->setIcon(QIcon::fromTheme("binarytools_add"));
    d->appendButton->setIconSize({ 16, 16 });
    d->appendButton->setFlat(true);
    d->appendButton->setToolTip(tr("append"));

    //Delete
    d->deleteButton = new DIconButton(this);
    d->deleteButton->setIcon(QIcon::fromTheme("binarytools_reduce"));
    d->deleteButton->setIconSize({ 16, 16 });
    d->deleteButton->setFlat(true);
    d->deleteButton->setToolTip(tr("reduce"));

    //Reset
    d->resetButton = new DIconButton(this);
    d->resetButton->setIcon(QIcon::fromTheme("binarytools_reset"));
    d->resetButton->setIconSize({ 16, 16 });
    d->resetButton->setFlat(true);
    d->resetButton->setToolTip(tr("reset"));

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(d->appendButton);
    btnLayout->addWidget(d->deleteButton);
    btnLayout->addWidget(d->resetButton);
    btnLayout->addStretch(1);

    d->vLayout->addLayout(btnLayout);
    d->vLayout->setContentsMargins(0, 0, 0, 0);

    connect(d->tableView->selectionModel(), &QItemSelectionModel::currentChanged, [=](const QModelIndex &current) {
        if (current.isValid() || d->tableView->selectionModel()->hasSelection()) {
            emit deleteSignal(true);
        } else {
            emit deleteSignal(false);
        }
    });

    initModel();

    connect(&d->model, &NameValueModel::focusIndex, this, &EnvironmentView::handleFocusIndex);
    connect(d->appendButton, &DPushButton::clicked, this, &EnvironmentView::appendRow);
    connect(d->deleteButton, &DPushButton::clicked, this, &EnvironmentView::deleteRow);
    connect(d->resetButton, &DPushButton::clicked, this, &EnvironmentView::initModel);
    connect(this, &EnvironmentView::deleteSignal, [=](bool enable) {
        d->deleteButton->setEnabled(enable);
    });
}

void EnvironmentView::disableDleteButton()
{
    d->deleteButton->setEnabled(false);
}

void EnvironmentView::handleFocusIndex(const QModelIndex &index)
{
    d->tableView->setCurrentIndex(index);
    d->tableView->setFocus();
    d->tableView->scrollTo(index, QTableView::PositionAtTop);
}

EnvironmentView::~EnvironmentView()
{
    if (d)
        delete d;
}

const QMap<QString, QVariant> EnvironmentView::getEnvironment()
{
    return d->model.items();
}

void EnvironmentView::appendRow()
{
    auto index = d->model.addItem();
    d->tableView->setCurrentIndex(index);
}

void EnvironmentView::deleteRow()
{
    QModelIndex index = d->tableView->currentIndex();
    d->model.removeItem(index);
}

void EnvironmentView::initModel()
{
    QMap<QString, QVariant> envs;
    QStringList keys = QProcessEnvironment::systemEnvironment().keys();
    for (auto key : keys) {
        QString value = QProcessEnvironment::systemEnvironment().value(key);
        envs.insert(key, value);
    }

    d->model.setItems(envs);
    emit deleteSignal(false);
}

void EnvironmentView::setValue(const QMap<QString, QVariant> &map)
{
    d->model.setItems(map);
}

QMap<QString, QVariant> EnvironmentView::defaultEnvironment()
{
    QMap<QString, QVariant> envs;
    QStringList keys = QProcessEnvironment::systemEnvironment().keys();
    for (auto key : keys) {
        QString value = QProcessEnvironment::systemEnvironment().value(key);
        envs.insert(key, value);
    }

    return envs;
}
