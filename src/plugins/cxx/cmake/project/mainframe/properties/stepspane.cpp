// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stepspane.h"

#include "services/project/projectservice.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QHeaderView>
#include <QTableView>
#include <QLineEdit>
#include <QLabel>

const QString ENABLE_ALL_ENV = StepsPane::tr("Enable All BuildSteps");
static const char *kBuildTitle = "Build:";
static const char *kBuildCommand = "cmake --build . --target ";

class StepsModelPrivate
{
    friend class StepsModel;

    QMap<QString, bool> targets;
};

StepsModel::StepsModel(QObject *parent)
    : QAbstractTableModel(parent)
    , d (new StepsModelPrivate())
{

}

StepsModel::~StepsModel()
{

}

int StepsModel::rowCount(const QModelIndex &) const
{
    return d->targets.keys().size();
}

int StepsModel::columnCount(const QModelIndex &) const
{
    return kColumnCount;
}

QVariant StepsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= d->targets.size())
        return QVariant();

    QString target = d->targets.keys().at(index.row());
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case kTarget:
            return target;
        case kPath:
            return "";
        default:
            break;
        }
    } else if (role == Qt::CheckStateRole) {
        int CHECK_BOX_COLUMN = 0;
        if (index.column() == CHECK_BOX_COLUMN) {
            if (d->targets.value(target)) {
                return Qt::Checked;
            } else {
                return Qt::Unchecked;
            }
        }
    } else if (role == Qt::TextAlignmentRole) {
         return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }
    return QVariant();
}

QVariant StepsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        case Qt::DisplayRole:
        {
            if (orientation == Qt::Horizontal) {
                switch (section) {
                case kTarget:
                    return "Target";
                case kPath:
                    return "Path";
                default:
                    break;
                }
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags StepsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    int CHECK_BOX_COLUMN = 0;
    if (index.column() == CHECK_BOX_COLUMN)
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}

bool StepsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (index.row() >= d->targets.size())
        return false;

    int nColumn = index.column();
    QString target = d->targets.keys().at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
    {
        return QAbstractTableModel::setData(index, value, role);
    }
    case Qt::CheckStateRole:
    {
        int CHECK_BOX_COLUMN = 0;
        if (nColumn == CHECK_BOX_COLUMN) {
            beginResetModel();

            bool bChecked = (value.toInt() == Qt::Checked);
            d->targets[target] = bChecked;

            if (bChecked) {
                foreach (auto key, d->targets.keys()) {
                    if(key != target) {
                        d->targets[key] = !bChecked;
                    }
                }
            }

            emit dataChanged(index, index);

            endResetModel();
            return true;
        }
        break;
    }
    default:
        return false;
    }
    return false;
}

void StepsModel::setData(const QMap<QString, bool> &data)
{
    beginResetModel();
    d->targets = data;
    endResetModel();
}

QString StepsModel::getSelectedTarget()
{
    QString selectedTarget;
    foreach (auto key, d->targets.keys()) {
        if(d->targets.value(key)) {
            selectedTarget = key;
        }
    }

    return selectedTarget;
}

class StepsPanePrivate
{
    friend class StepsPane;

    QLineEdit *toolArguments{nullptr};
    QLabel *buildLabel{nullptr};
    StepsModel *model{nullptr};
};

StepsPane::StepsPane(QWidget *parent)
    : QWidget(parent)
    , d(new StepsPanePrivate)
{
    setupUi();

    updateSummaryText();
}

StepsPane::~StepsPane()
{
    if(d) {
        delete d;
    }
}

void StepsPane::setupUi()
{
    setAutoFillBackground(true);

    QVBoxLayout *vLayout = new QVBoxLayout(this);

    d->buildLabel = new QLabel(this);
    d->buildLabel->setText(QString("Build:").append(kBuildCommand));

    QTableView *tableView = new QTableView();
    tableView->setShowGrid(false);
    QHeaderView* headerView = tableView->horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::ResizeToContents);
    headerView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->verticalHeader()->hide();

    d->model = new StepsModel();
    tableView->setModel(d->model);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    d->toolArguments = new QLineEdit(this);
    d->toolArguments->setPlaceholderText(tr("Input your arguments."));
    QLabel *label = new QLabel(tr("Tool arguments:"), this);
    hLayout->addWidget(label);
    hLayout->addWidget(d->toolArguments);

    vLayout->setMargin(0);
    vLayout->addWidget(d->buildLabel);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(tableView);

    connect(d->toolArguments, &QLineEdit::textEdited, this, &StepsPane::toolArgumentsEdited);
    connect(d->model, &StepsModel::dataChanged, this, &StepsPane::dataChanged);

    setLayout(vLayout);
}

void StepsPane::toolArgumentsEdited()
{
    updateSummaryText();
}

void StepsPane::dataChanged()
{
    updateSummaryText();
}

QString StepsPane::getCombinedBuildText()
{
    QString head = kBuildTitle;
    QString defaultCommand = kBuildCommand;
    QString arguments = d->toolArguments->text();

    QString buildText = head + defaultCommand + d->model->getSelectedTarget() + (arguments.isEmpty() ? "" : " -- " + arguments);
    return buildText;
}

void StepsPane::updateSummaryText()
{
    d->buildLabel->setText(getCombinedBuildText());
}

void StepsPane::setValues(const config::StepItem &item)
{
    d->toolArguments->setText(item.arguments);

    QMap<QString, bool> data;
    foreach (auto targetName, item.targetList) {
        data.insert(targetName, targetName == item.targetName ? true : false);
    }

    d->model->setData(data);
    updateSummaryText();
}

void StepsPane::getValues(config::StepItem &item)
{
    item.arguments = d->toolArguments->text();
    item.targetName = d->model->getSelectedTarget();
}
