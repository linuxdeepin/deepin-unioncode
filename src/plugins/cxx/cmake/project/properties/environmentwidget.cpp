// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environmentwidget.h"

#include <DFrame>
#include <DCheckBox>
#include <DTableView>
#include <DIconButton>
#include <DPushButton>

#include <QVBoxLayout>
#include <QHeaderView>

DWIDGET_USE_NAMESPACE

const char kLoggingRules[] = "QT_LOGGING_RULES";

class EnvironmentModelPrivate
{
    friend class EnvironmentModel;
    QMap<QString, QString> envs;
};

EnvironmentModel::EnvironmentModel(QObject *parent)
    : QAbstractTableModel(parent)
    , d (new EnvironmentModelPrivate())
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
    return kColumnCount;
}

QVariant EnvironmentModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        auto var = d->envs.keys()[index.row()];
        switch (index.column()) {
        case kVaribale:
            return var;
        case kValue:
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

    const QString oldName = data(this->index(index.row(), 0, QModelIndex()), Qt::EditRole).toString();
    const QString oldValue = data(this->index(index.row(), 1, QModelIndex()), Qt::EditRole).toString();
    QMap<QString, QString> map = d->envs;
    if (index.column() == kVaribale) {
        const QString newName = value.toString();
        if (newName.isEmpty() || newName.contains("=") || map.contains(newName))
            return false;
        map.remove(oldName);
        map.insert(value.toString(), oldValue);
    } else if (index.column() == kValue) {
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
        case kVaribale:
            return QObject::tr("Variable");
        case kValue:
            return QObject::tr("Value");
        default:
            break;
        }
    }
    return {};
}

QModelIndex EnvironmentModel::append(const QString &key, const QString &value)
{
    auto list = d->envs.keys();
    list.append(key);
    qSort(list);
    int pos = list.indexOf(key);

    beginInsertRows(QModelIndex(), pos, pos);
    d->envs.insert(key, value);
    endInsertRows();
    emit dataChanged(index(pos, 0), index(pos, 1));
    return index(pos, 0);
}

void EnvironmentModel::remove(QModelIndex &index)
{
    if (d->envs.keys().isEmpty() || index.row() < 0 || index.row() >= d->envs.count())
        return;
    int row = index.row();
    beginRemoveRows(QModelIndex(), row, row);
    QString key = d->envs.keys()[index.row()];
    d->envs.remove(key);
    endRemoveRows();
    emit dataChanged(index, index);
}

void EnvironmentModel::update(const QMap<QString, QString> &data)
{
    beginResetModel();
    d->envs.clear();
    d->envs = data;
    endResetModel();
    emit dataChanged(index(0, 0), index(d->envs.count() - 1, 1));
}

const QMap<QString, QString> EnvironmentModel::getEnvironment() const
{
    return d->envs;
}

class EnvironmentWidgetPrivate
{
    friend class EnvironmentWidget;

    QVBoxLayout *vLayout{nullptr};
    DTableView *tableView{nullptr};
    DCheckBox *enableEnvCB{nullptr};
    DCheckBox *enableQDebugLevelCB{nullptr};
    DIconButton *appendButton = nullptr;
    DIconButton *deleteButton = nullptr;
    DIconButton *resetButton = nullptr;
    EnvironmentModel *model{nullptr};

    config::EnvironmentItem *envShadow{nullptr};
};


EnvironmentWidget::EnvironmentWidget(QWidget *parent, EnvType type)
    : DFrame(parent)
    , d(new EnvironmentWidgetPrivate)
{
    setAutoFillBackground(true);

    if (!d->vLayout)
        d->vLayout = new QVBoxLayout();
    this->setLayout(d->vLayout);

    if (!d->tableView) {
        d->tableView = new DTableView(this);
        d->tableView->setAlternatingRowColors(true);
        d->tableView->setFrameShape(QFrame::NoFrame);

        // Initialize view
        d->tableView->setShowGrid(false);
        QHeaderView* headerView = d->tableView->horizontalHeader();
        headerView->setDefaultAlignment(Qt::AlignLeft);
        headerView->setSectionResizeMode(QHeaderView::Stretch);

        d->tableView->verticalHeader()->hide();
    }

    if (!d->model)
        d->model = new EnvironmentModel();
    connect(d->model, &EnvironmentModel::dataChanged, this, &EnvironmentWidget::envUpdated);

    d->tableView->setModel(d->model);

    // add enable env check box.
    if (!d->enableEnvCB)
        d->enableEnvCB = new DCheckBox(this);

    connect(d->enableEnvCB, &DCheckBox::clicked, [this](){
        if (d->envShadow)
            d->envShadow->enable = d->enableEnvCB->isChecked();
    });

    d->enableEnvCB->setText(tr("Enable All Environment"));
    d->enableEnvCB->setChecked(true);

    // add enable qdebug level check box.
    if (!d->enableQDebugLevelCB)
        d->enableQDebugLevelCB = new DCheckBox(this);
    if (type == EnvType::BuildCfg)
        d->enableQDebugLevelCB->hide();

    connect(d->enableQDebugLevelCB, &DCheckBox::stateChanged, this, &EnvironmentWidget::onEnableQDebugLevel);

    d->enableQDebugLevelCB->setText(tr("Enable Qt Debug Level"));
    d->enableQDebugLevelCB->setChecked(false);

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
    btnLayout->setSpacing(5);
    btnLayout->setContentsMargins(5, 0, 0, 0);

    connect(d->appendButton, &DPushButton::clicked, this, &EnvironmentWidget::appendRow);
    connect(d->deleteButton, &DPushButton::clicked, this, &EnvironmentWidget::deleteRow);
    connect(d->resetButton, &DPushButton::clicked, this, &EnvironmentWidget::initModel);

    // instert to layout.
    d->vLayout->setSpacing(0);
    d->vLayout->setMargin(0);
    d->vLayout->addWidget(d->tableView);
    d->vLayout->addLayout(btnLayout);
    d->vLayout->addWidget(d->enableEnvCB);
    d->vLayout->addWidget(d->enableQDebugLevelCB);
}

EnvironmentWidget::~EnvironmentWidget()
{
    if(d)
        delete d;
}

void EnvironmentWidget::appendRow()
{
    auto index = d->model->append("<KEY>", "<VALUE>");
    d->tableView->setCurrentIndex(index);
}

void EnvironmentWidget::deleteRow()
{
    QModelIndex index = d->tableView->currentIndex();
    d->model->remove(index);
}

void EnvironmentWidget::initModel()
{
    QMap<QString, QString> envs;
    QStringList keys = QProcessEnvironment::systemEnvironment().keys();
    for (auto key : keys) {
        QString value = QProcessEnvironment::systemEnvironment().value(key);
        envs.insert(key, value);
    }
    d->model->update(envs);
}

void EnvironmentWidget::getValues(config::EnvironmentItem &env)
{
    env.enable = d->enableEnvCB->isChecked();
    env.environments = d->model->getEnvironment();
    env.setQDebugLevel(d->enableQDebugLevelCB->isChecked());
}

void EnvironmentWidget::setValues(const config::EnvironmentItem &env)
{
    d->enableEnvCB->setChecked(env.enable);
    d->enableQDebugLevelCB->setChecked(env.isQDebugLevelEnable());
    d->model->update(env.environments);
}

void EnvironmentWidget::updateEnvList(config::EnvironmentItem *env)
{
    d->envShadow = env;
    d->enableEnvCB->setChecked(env->enable);
    d->enableQDebugLevelCB->setChecked(env->isQDebugLevelEnable());
    d->model->update(env->environments);
}

void EnvironmentWidget::onEnableQDebugLevel()
{
    if (!d->envShadow)
        return;

    bool isQDebugLevelEnabled = d->enableQDebugLevelCB->isChecked();
    d->envShadow->setQDebugLevel(isQDebugLevelEnabled);

    d->model->update(d->envShadow->environments);
}


