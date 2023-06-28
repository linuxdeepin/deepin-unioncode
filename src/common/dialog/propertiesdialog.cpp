// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "propertiesdialog.h"
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

PropertiesDialog::PropertiesDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
}

bool PropertiesDialog::insertPropertyPanel(const QString &itemName, PageWidget *panel)
{
    widgts.insert(itemName, panel);
    leftBarModel->setStringList(leftBarModel->stringList() << itemName);
    int index = stackWidget->count();
    stackWidget->insertWidget(index, panel);
    leftBarValues.push_back(itemName);

    if (index >= 0 && !leftBarValues.isEmpty()) {
        stackWidget->setCurrentIndex(0);
        headTitle->setText(leftBarValues.at(0));
    }

    return true;
}

void PropertiesDialog::showPropertyPanel(const QString &itemName, const QString &tabName)
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

void PropertiesDialog::slotLeftBarClicked(const QModelIndex &index)
{
    qInfo() << "The selected item is :" << index.data().toString();
    QString itemName = index.data().toString();
    auto widget = widgts.value(itemName);
    stackWidget->setCurrentWidget(widget);

    // Update head title.
    headTitle->setText(itemName);
    widget->readConfig();
}

void PropertiesDialog::slotFilterText(const QString &text)
{
    QString filterText = text.trimmed();
    if (filterText.isEmpty()) {
        leftBarModel->setStringList(leftBarValues);
        return;
    }

    QStringList tempList;
    foreach (auto str, leftBarValues) {
        QString upperStr = str.toUpper();
        QString uppprText = text.toUpper();
        if (upperStr.contains(uppprText)) {
            tempList.push_back(str);
        }
    }

    leftBarModel->setStringList(tempList);
}

void PropertiesDialog::setupUi(QDialog *Dialog)
{
    Dialog->resize(1000, 650);
    setWindowTitle(tr("Project Properties"));

    // Center layout.
    auto mainLayout = new QHBoxLayout(Dialog);
    mainLayout->setSpacing(6);
    mainLayout->setContentsMargins(11, 11, 11, 11);

    // Left layout.
    auto leftLayout = new QVBoxLayout(Dialog);
    leftLayout->setSpacing(6);
    filterEdit = new QLineEdit(Dialog);
    filterEdit->setPlaceholderText(tr("Filter"));
    connect(filterEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(slotFilterText(const QString &)));
    leftLayout->addWidget(filterEdit);

    leftSideBar = new QListView(Dialog);
    leftSideBar->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leftBarModel = new QStringListModel(leftSideBar);
    leftSideBar->setModel(leftBarModel);
    connect(leftSideBar, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(slotLeftBarClicked(const QModelIndex &)));

    leftLayout->addWidget(leftSideBar);

    // Right layout.
    auto rightLayout = new QVBoxLayout(Dialog);
    rightLayout->setSpacing(6);
    headTitle = new QLabel(Dialog);

    rightLayout->addWidget(headTitle);

    stackWidget = new QStackedWidget(Dialog);
    rightLayout->addWidget(stackWidget);

    auto buttonLayout = new QHBoxLayout(Dialog);
    buttonLayout->setSpacing(6);
    auto horizontalSpacer = new QSpacerItem(40, 30, QSizePolicy::Expanding, QSizePolicy::Minimum);

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

void PropertiesDialog::saveAllConfig()
{
    for (int index = 0; index < stackWidget->count(); index++)
    {
        PageWidget* widget = dynamic_cast<PageWidget*>(stackWidget->widget(index));
        if (widget) {
            widget->saveConfig();
        }
    }

    accept();
}

void PropertiesDialog::saveSingleConfig()
{
    int index = stackWidget->currentIndex();
    if (index >= 0 && index < stackWidget->count())
    {
        PageWidget* widget = dynamic_cast<PageWidget*>(stackWidget->widget(index));
        if (widget) {
            widget->saveConfig();
        }
    }
}

void PropertiesDialog::readConfig()
{
    for (int index = 0; index < stackWidget->count(); index++)
    {
        PageWidget* widget = dynamic_cast<PageWidget*>(stackWidget->widget(index));
        if (widget) {
            widget->readConfig();
        }
    }
}
