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
#include "runtimewidget.h"
#include "configurewidget.h"
#include "environmentwidget.h"
#include "common/common.h"
#include "stepspane.h"

#include <QListWidget>
#include <QSplitter>
#include <QComboBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QStyleFactory>

static RuntimeWidget *ins{nullptr};
class RuntimeWidgetPrivate
{
    friend class RuntimeWidget;
    QVBoxLayout *leftLayout{nullptr};
    QVBoxLayout *rightLayout{nullptr};
    QGroupBox *folderGroupBox{nullptr};
    QComboBox *openFolderBox{nullptr};
    QPushButton *openNewFolder{nullptr};
    ConfigureWidget *configWidget{nullptr};
};

RuntimeWidget *RuntimeWidget::instance()
{
    if (!ins)
        ins = new RuntimeWidget();
    return ins;
}

RuntimeWidget::RuntimeWidget(QWidget *parent)
    : QSplitter (parent)
    , d(new RuntimeWidgetPrivate())
{
    d->leftLayout = new QVBoxLayout();
    d->folderGroupBox = new QGroupBox(tr("Open Folder"));
    d->openFolderBox = new QComboBox();
    d->openNewFolder = new QPushButton(tr("Open New Folder"));
    d->folderGroupBox->setLayout(d->leftLayout);
    d->leftLayout->addWidget(d->openFolderBox, 0, Qt::AlignTop);
    d->leftLayout->addWidget(d->openNewFolder, 0, Qt::AlignTop);
    d->leftLayout->addStretch();
    d->openFolderBox->addItems({"unioncode", "dde-file-manager"});// test project name
    d->configWidget = new ConfigureWidget(this);
    auto buldStepPane = new CollapseWidget("Build Steps", new StepsPane(StepsPane::kBuild));
    d->configWidget->addCollapseWidget(buldStepPane);
    d->configWidget->addCollapseWidget(new CollapseWidget("Clean Steps", new StepsPane(StepsPane::kClean)));
    d->configWidget->addCollapseWidget(new CollapseWidget("Runtime Env", new EnvironmentWidget));
    addWidget(d->folderGroupBox);
    addWidget(d->configWidget);
    setStretchFactor(1, 3);
    setChildrenCollapsible(false);

    // expand environment tree widget.
    buldStepPane->setChecked(false);
}

RuntimeWidget::~RuntimeWidget()
{
    if (d)
        delete d;
}
