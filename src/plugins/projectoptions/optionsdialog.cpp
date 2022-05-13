/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#include "optionsdialog.h"
#include "common/widget/pagewidget.h"

#include <QtDebug>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QStringListModel>

OptionsDialog::OptionsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
}

bool OptionsDialog::insertOptionPanel(const QString &itemName, QWidget *panel)
{
    widgts.insert(itemName, panel);
    leftBarModel->setStringList(leftBarModel->stringList() << itemName);
    int index = stackWidget->count();
    stackWidget->insertWidget(index, panel);

    return true;
}

void OptionsDialog::showProjectOptionsDlg(const QString &itemName, const QString &tabName)
{
    Q_UNUSED(tabName)

    // Set leftbar item seleted.
    int index = leftBarModel->stringList().indexOf(itemName);
    if (index >= 0) {
        leftSideBar->selectionModel()->select(leftBarModel->index(index), QItemSelectionModel::Select);

        // Update right panel.
        auto widget = widgts.value(itemName);
        stackWidget->setCurrentWidget(widget);

        headTitle->setText(itemName);
    }

    this->exec();
}

void OptionsDialog::slotLeftBarClicked(const QModelIndex &index)
{
    qInfo() << "The selected item is :" << index.data().toString();
    QString itemName = index.data().toString();
    auto widget = widgts.value(itemName);
    stackWidget->setCurrentWidget(widget);

    // Update head title.
    headTitle->setText(itemName);
}

void OptionsDialog::setupUi(QDialog *Dialog)
{
    Dialog->resize(1000, 600);
    setWindowTitle(tr("Project Options"));

    // Center layout.
    auto mainLayout = new QHBoxLayout(Dialog);
    mainLayout->setSpacing(6);
    mainLayout->setContentsMargins(11, 11, 11, 11);

    // Left layout.
    auto leftLayout = new QVBoxLayout(Dialog);
    leftLayout->setSpacing(6);
    filterEdit = new QLineEdit(Dialog);

    leftLayout->addWidget(filterEdit);

    leftSideBar = new QListView(Dialog);
    leftBarModel = new QStringListModel(leftSideBar);
    leftSideBar->setModel(leftBarModel);
    connect(leftSideBar, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(slotLeftBarClicked(const QModelIndex &)));

    leftLayout->addWidget(leftSideBar);

    // Right layout.
    auto rightLayout = new QVBoxLayout(Dialog);
    rightLayout->setSpacing(6);
    headTitle = new QLabel(Dialog);
    headTitle->setText(tr("Head Title"));

    rightLayout->addWidget(headTitle);

    stackWidget = new QStackedWidget(Dialog);
    rightLayout->addWidget(stackWidget);

    auto buttonLayout = new QHBoxLayout(Dialog);
    buttonLayout->setSpacing(6);
    auto horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    buttonLayout->addItem(horizontalSpacer);

    auto okBtn = new QPushButton(tr("OK"), Dialog);
    connect(okBtn, SIGNAL(clicked()), this, SLOT(saveAllConfig()));

    auto cancelBtn = new QPushButton(tr("Cancel"), Dialog);
    connect(cancelBtn, &QPushButton::clicked, [this] {
        // TODO(Mozart)
        this->close();
    });
    auto applyBtn = new QPushButton(tr("Apply"), Dialog);
    connect(applyBtn, SIGNAL(clicked()), this, SLOT(saveSingleConfig()));

    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(applyBtn);

    rightLayout->addLayout(buttonLayout);

    // Insert left & right layout to main layout.
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    mainLayout->setStretch(0, 1);
    mainLayout->setStretch(1, 4);
}

void OptionsDialog::saveAllConfig()
{
    for (int nIndex = 0; nIndex < stackWidget->count(); nIndex++)
    {
        PageWidget* pWidget = dynamic_cast<PageWidget*>(stackWidget->widget(nIndex));
        if (pWidget) {
            pWidget->saveConfig();
        }
    }

    accept();
}

void OptionsDialog::saveSingleConfig()
{
    int nIndex = stackWidget->currentIndex();
    if (nIndex > 0 && nIndex < stackWidget->count())
    {
        PageWidget* pWidget = dynamic_cast<PageWidget*>(stackWidget->widget(nIndex));
        if (pWidget) {
            pWidget->saveConfig();
        }
    }
}
