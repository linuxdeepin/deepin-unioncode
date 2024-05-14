// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environmentview.h"
#include "binarytoolsconfigview.h"

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
class EnvironmentModelPrivate
{
    friend class EnvironmentModel;
    QMap<QString, QVariant> envs;
};

EnvironmentModel::EnvironmentModel(QObject *parent)
    : QAbstractTableModel(parent), d(new EnvironmentModelPrivate())
{
}

EnvironmentModel::~EnvironmentModel()
{
    if (d)
        delete d;
}

int EnvironmentModel::rowCount(const QModelIndex &) const
{
    return d->envs.keys().size();
}

int EnvironmentModel::columnCount(const QModelIndex &) const
{
    return ColumnCount;
}

QVariant EnvironmentModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        auto var = d->envs.keys()[index.row()];
        switch (index.column()) {
        case Key:
            return var;
        case Value:
            return d->envs.value(var);
        default:
            break;
        }
    }
    return {};
}

bool EnvironmentModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return true;

    const QString oldName = data(this->index(index.row(), 0, QModelIndex())).toString();
    const QString oldValue = data(this->index(index.row(), 1, QModelIndex()), Qt::EditRole).toString();
    QMap<QString, QVariant> map = d->envs;
    if (index.column() == Key) {
        const QString newName = value.toString();
        if (newName.isEmpty() || newName.contains("="))
            return false;
        if (map.contains(newName) || newName.isEmpty())
            return false;
        map.remove(oldName);
        map.insert(value.toString(), oldValue);
    } else if (index.column() == Value) {
        const QString stringValue = value.toString();
        auto var = map.keys()[index.row()];
        map[var] = stringValue;
    }
    update(map);
    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags EnvironmentModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}

QVariant EnvironmentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case Key:
            return QObject::tr("Variable");
        case Value:
            return QObject::tr("Value");
        default:
            break;
        }
    }
    return {};
}

QModelIndex EnvironmentModel::append(const QString &key, const QVariant &value)
{
    beginResetModel();
    d->envs.insert(key, value);
    endResetModel();

    for (int row = 0; row < d->envs.count(); row++) {
        if (index(row, 0).data() == key)
            return index(row, 0);
    }
}

void EnvironmentModel::remove(QModelIndex &index)
{
    if (d->envs.keys().isEmpty() || index.row() < 0)
        return;

    beginResetModel();
    QString key = d->envs.keys()[index.row()];
    d->envs.remove(key);
    endResetModel();
}

void EnvironmentModel::update(const QMap<QString, QVariant> &data)
{
    beginResetModel();
    d->envs.clear();
    d->envs = data;
    endResetModel();
}

const QMap<QString, QVariant> EnvironmentModel::getEnvironment() const
{
    return d->envs;
}

class EnvironmentViewPrivate
{
    friend class EnvironmentView;

    QVBoxLayout *vLayout = nullptr;
    DTableView *tableView = nullptr;
    EnvironmentModel *model = nullptr;
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

    if (!d->model)
        d->model = new EnvironmentModel();

    d->tableView->setModel(d->model);

    //append
    d->appendButton = new DIconButton(this);
    d->appendButton->setIcon(QIcon::fromTheme("binarytools_add"));
    d->appendButton->setIconSize({16, 16});
    d->appendButton->setFlat(true);
    d->appendButton->setToolTip(tr("append"));

    //Delete
    d->deleteButton = new DIconButton(this);
    d->deleteButton->setIcon(QIcon::fromTheme("binarytools_reduce"));
    d->deleteButton->setIconSize({16, 16});
    d->deleteButton->setFlat(true);
    d->deleteButton->setToolTip(tr("reduce"));

    //Reset
    d->resetButton = new DIconButton(this);
    d->resetButton->setIcon(QIcon::fromTheme("binarytools_reset"));
    d->resetButton->setIconSize({16, 16});
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

EnvironmentView::~EnvironmentView()
{
    if (d)
        delete d;
}

const QMap<QString, QVariant> EnvironmentView::getEnvironment()
{
    return d->model->getEnvironment();
}

void EnvironmentView::appendRow()
{
    auto index = d->model->append("<KEY>", "<VALUE>");
    d->tableView->setCurrentIndex(index);
}

void EnvironmentView::deleteRow()
{
    QModelIndex index = d->tableView->currentIndex();
    d->model->remove(index);
}

void EnvironmentView::initModel()
{
    QMap<QString, QVariant> envs;
    QStringList keys = QProcessEnvironment::systemEnvironment().keys();
    for (auto key : keys) {
        QString value = QProcessEnvironment::systemEnvironment().value(key);
        envs.insert(key, value);
    }

    d->model->update(envs);
    emit deleteSignal(false);
}

void EnvironmentView::setValue(const QMap<QString, QVariant> &map)
{
    d->model->update(map);
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
