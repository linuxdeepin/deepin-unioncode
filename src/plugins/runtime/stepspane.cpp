/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "stepspane.h"
#include "targetsmanager.h"

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

class BuildStepsModel;
class BuildStepsPanePrivate
{
    friend class StepsPane;
    QVBoxLayout *vLayout = nullptr;
    QTableView *tableView = nullptr;
    QLineEdit *toolArguments = nullptr;
    BuildStepsModel *model = nullptr;
    QLabel *buildLabel = nullptr;
    StepsPane::StepType stepType;
};

class BuildStepsModel : public QAbstractTableModel
{
public:
    enum ColumnType
    {
        kCheckBox,
        kTarget,
        kPath,
        kColumnCount
    };

    explicit BuildStepsModel(QObject *parent = nullptr)
        : QAbstractTableModel(parent)
    {
    }

    void setStepType(StepsPane::StepType _stepType)
    {
        stepType = _stepType;
    }

    int rowCount(const QModelIndex &) const override
    {
        return TargetsManager::instance()->getTargetNamesList().size();
    }

    int columnCount(const QModelIndex &) const override
    {
        return kColumnCount;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        const QString &target = TargetsManager::instance()->getTargetNamesList()[index.row()];
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            switch (index.column()) {
            case kTarget:
                return target;
            case kPath:
                return "";
            default:
                ; // do nothing
            }
        } else if (role == Qt::CheckStateRole) {
            int CHECK_BOX_COLUMN = 0;
            if (index.column() == CHECK_BOX_COLUMN) {
                if (target == getActiveTarget()) {
                    return Qt::Checked;
                } else {
                    return Qt::Unchecked;
                }
            }
        } else if (role == Qt::TextAlignmentRole) {
             return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
        return {};
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
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
                        ; // do nothing.
                    }
                }
            }
        }
        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (!index.isValid())
            return QAbstractItemModel::flags(index);

        Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        int CHECK_BOX_COLUMN = 0;
        if (index.column() == CHECK_BOX_COLUMN)
            flags |= Qt::ItemIsUserCheckable;

        return flags;
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        if (!index.isValid())
            return false;

        int nColumn = index.column();
        const QString &target = TargetsManager::instance()->getTargetNamesList().at(index.row());
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
                updateActiveTarget(bChecked, target);
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

private:
    QString getActiveTarget() const
    {
        if (StepsPane::kBuild == stepType) {
            return TargetsManager::instance()->getActiveBuildTargetName();
        }
        return TargetsManager::instance()->getActiveCleanTargetName();
    }

    void updateActiveTarget(bool bChecked, const QString &target)
    {
        QString activeTarget;
        if (bChecked) {
            activeTarget = target;
        }

        if (StepsPane::kBuild == stepType) {
            TargetsManager::instance()->updateActiveBuildTarget(target);
        } else {
            TargetsManager::instance()->updateActiveCleanTarget(target);
        }
    }

    StepsPane::StepType stepType = StepsPane::kBuild;
};

StepsPane::StepsPane(StepType stepType, QWidget *parent)
    : QWidget (parent)
    , d(new BuildStepsPanePrivate)
{
    d->stepType = stepType;

    setupUi();

    updateSummaryText();
}

StepsPane::~StepsPane()
{
    if(d) {
        delete d;
    }
}

QString StepsPane::getActiveTarget() const
{
    if (StepsPane::kBuild == d->stepType) {
        return TargetsManager::instance()->getActiveBuildTargetName();
    }
    return TargetsManager::instance()->getActiveCleanTargetName();
}

void StepsPane::setupUi()
{
    setAutoFillBackground(true);

    if (!d->vLayout)
        d->vLayout = new QVBoxLayout(this);

    if (!d->buildLabel) {
        d->buildLabel = new QLabel(this);
        d->buildLabel->setText(QString("Build:").append(kBuildCommand));
    }

    if (!d->tableView) {
        d->tableView = new QTableView();

        // Initialize view
        d->tableView->setShowGrid(false);
        QHeaderView* headerView = d->tableView->horizontalHeader();
        headerView->setSectionResizeMode(QHeaderView::ResizeToContents);
        headerView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    if (!d->model) {
        d->model = new BuildStepsModel();
        d->model->setStepType(d->stepType);
    }

    d->tableView->setModel(d->model);

    QHBoxLayout *hLay = new QHBoxLayout(this);
    if (!d->toolArguments) {
        d->toolArguments = new QLineEdit(this);
        d->toolArguments->setPlaceholderText(tr("Input your arguments."));
        QLabel *label = new QLabel(this);
        label->setText(tr("Tool arguments:"));

        hLay->addWidget(label);
        hLay->addWidget(d->toolArguments);
    }

    this->setLayout(d->vLayout);
    d->vLayout->setMargin(0);
    d->vLayout->addWidget(d->buildLabel);
    d->vLayout->addLayout(hLay);
    d->vLayout->addWidget(d->tableView);

    connect(d->toolArguments, &QLineEdit::textEdited, this, &StepsPane::toolArgumentsEdited);
    connect(d->model, &BuildStepsModel::dataChanged, this, &StepsPane::dataChanged);
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
    QString target = getActiveTarget();
    QString arguments = d->toolArguments->text();

    QString buildText = head + defaultCommand + target + (arguments.isEmpty() ? "" : " --" + arguments);
    return buildText;
}

void StepsPane::updateSummaryText()
{
    d->buildLabel->setText(getCombinedBuildText());
}

