/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "environmentwidget.h"

#include "projectparser.h"

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
                return "Variable";
            case kValue:
                return "Value";
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
    : QWidget (parent)
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
    }

    if (!d->model)
        d->model = new EnvironmentModel();

    d->tableView->setModel(d->model);

    if (!d->checkBox)
        d->checkBox = new QCheckBox();
    d->checkBox->setText(ENABLE_ALL_ENV);
    d->checkBox->setChecked(true);
    d->vLayout->setSpacing(0);
    d->vLayout->setMargin(0);
    d->vLayout->addWidget(d->checkBox);
    d->vLayout->addWidget(d->tableView);
}

EnvironmentWidget::~EnvironmentWidget()
{
    if(d)
        delete d;
}
