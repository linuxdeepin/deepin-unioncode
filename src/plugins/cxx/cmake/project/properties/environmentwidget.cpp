// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environmentwidget.h"

#include "base/baseitemdelegate.h"
#include "common/util/namevaluemodel.h"

#include <DFrame>
#include <DCheckBox>
#include <DTableView>
#include <DIconButton>
#include <DPushButton>

#include <QVBoxLayout>
#include <QHeaderView>

DWIDGET_USE_NAMESPACE

const char kLoggingRules[] = "QT_LOGGING_RULES";

class EnvironmentWidgetPrivate
{
    friend class EnvironmentWidget;

    QVBoxLayout *vLayout{nullptr};
    DTableView *tableView{nullptr};
    DCheckBox *enableEnvCB{nullptr};
    DIconButton *appendButton = nullptr;
    DIconButton *deleteButton = nullptr;
    DIconButton *resetButton = nullptr;
    NameValueModel model;

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

    connect(&d->model, &NameValueModel::dataChanged, this, &EnvironmentWidget::envUpdated);
    connect(&d->model, &NameValueModel::focusIndex, this, &EnvironmentWidget::handleFocusIndex);

    d->tableView->setModel(&d->model);
    d->tableView->setItemDelegate(new BaseItemDelegate(d->tableView));

    // add enable env check box.
    if (!d->enableEnvCB)
        d->enableEnvCB = new DCheckBox(this);

    connect(d->enableEnvCB, &DCheckBox::clicked, [this](){
        if (d->envShadow)
            d->envShadow->enable = d->enableEnvCB->isChecked();
    });

    d->enableEnvCB->setText(tr("Enable All Environment"));
    d->enableEnvCB->setChecked(true);

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
    d->vLayout->setContentsMargins(0, 0, 0, 0);
    d->vLayout->addWidget(d->tableView);
    d->vLayout->addLayout(btnLayout);
    d->vLayout->addWidget(d->enableEnvCB);
}

EnvironmentWidget::~EnvironmentWidget()
{
    if(d)
        delete d;
}

void EnvironmentWidget::appendRow()
{
    auto index = d->model.addItem();
    d->tableView->setCurrentIndex(index);
}

void EnvironmentWidget::deleteRow()
{
    QModelIndex index = d->tableView->currentIndex();
    d->model.removeItem(index);
}

void EnvironmentWidget::initModel()
{
    QVariantMap envs;
    QStringList keys = QProcessEnvironment::systemEnvironment().keys();
    for (auto key : keys) {
        QString value = QProcessEnvironment::systemEnvironment().value(key);
        envs.insert(key, value);
    }
    d->model.setItems(envs);
}

void EnvironmentWidget::getValues(config::EnvironmentItem &env)
{
    env.enable = d->enableEnvCB->isChecked();
    env.environments = d->model.items();
}

void EnvironmentWidget::setValues(const config::EnvironmentItem &env)
{
    d->enableEnvCB->setChecked(env.enable);
    d->model.setItems(env.environments);
}

void EnvironmentWidget::updateEnvList(config::EnvironmentItem *env)
{
    d->envShadow = env;
    d->enableEnvCB->setChecked(env->enable);
    d->model.setItems(env->environments);
}

void EnvironmentWidget::handleFocusIndex(const QModelIndex &index)
{
    d->tableView->setCurrentIndex(index);
    d->tableView->setFocus();
    d->tableView->scrollTo(index, QTableView::PositionAtTop);
}
