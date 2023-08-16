// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environmentwidget.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QProcessEnvironment>
#include <QHeaderView>


const QString ENABLE_ALL_ENV = EnvironmentWidget::tr("Enable All Environment");

class EnvironmentModel;
class EnvironmentWidgetPrivate
{
    friend class EnvironmentWidget;
    QVBoxLayout *vLayout = nullptr;
    QTableView *tableView = nullptr;
    QCheckBox *checkBox = nullptr;
    EnvironmentModel *model = nullptr;
};

class EnvironmentModel : public QAbstractTableModel
{
public:
    enum ColumnType
    {
        kVaribale,
        kValue,
        kColumnCount
    };

    explicit EnvironmentModel(QObject *parent = nullptr)
        : QAbstractTableModel(parent)
        , envs(QProcessEnvironment::systemEnvironment())
    {
    }

    int rowCount(const QModelIndex &) const override
    {
        return envs.keys().size();
    }

    int columnCount(const QModelIndex &) const override
    {
        return kColumnCount;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            auto var = envs.keys()[index.row()];
            switch (index.column()) {
            case kVaribale:
                return var;
            case kValue:
                return envs.value(var);
            default:
                ; // do nothing
            }
        }
        return {};
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case kVaribale:
                return QObject::tr("Variable");
            case kValue:
                return QObject::tr("Value");
            default:
                ; // do nothing.
            }
        }
        return {};
    }

    void append(const QString &key, const QString &value)
    {
        beginInsertRows({}, envs.keys().count(), envs.keys().count());
        envs.insert(key, value);
        endInsertRows();
    }

private:
    QProcessEnvironment envs;
};

EnvironmentWidget::EnvironmentWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new EnvironmentWidgetPrivate)
{
    setAutoFillBackground(true);

    if (!d->vLayout)
        d->vLayout = new QVBoxLayout();
    this->setLayout(d->vLayout);

    if (!d->tableView) {
        d->tableView = new QTableView();

        // Initialize view
        d->tableView->setShowGrid(false);
        QHeaderView* headerView = d->tableView->horizontalHeader();
        headerView->setSectionResizeMode(QHeaderView::ResizeToContents);
        d->tableView->verticalHeader()->hide();
    }

    if (!d->model)
        d->model = new EnvironmentModel();

    d->tableView->setModel(d->model);

    if (!d->checkBox)
        d->checkBox = new QCheckBox();
    d->checkBox->setText(ENABLE_ALL_ENV);
    d->checkBox->setChecked(true);
    d->vLayout->setSpacing(0);
    d->vLayout->setMargin(5);
    d->vLayout->addWidget(d->tableView);
    d->vLayout->addWidget(d->checkBox);
}

EnvironmentWidget::~EnvironmentWidget()
{
    if(d)
        delete d;
}
