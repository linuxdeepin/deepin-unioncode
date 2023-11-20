// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionsdialog.h"
#include "common/widget/pagewidget.h"
#include "services/option/optionservice.h"

#include <DTitlebar>
#include <DFrame>

#include <QtDebug>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QScrollBar>
#include <QStringListModel>

static const int labelHeight = 30;

using namespace dpfservice;
OptionsDialog::OptionsDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    setupUi(this);
}

bool OptionsDialog::insertOptionPanel(const QString &itemName, PageWidget *panel)
{
    widgets.insert(itemName, panel);
    leftBarModel->setStringList(leftBarModel->stringList() << itemName);

    DLabel *headTitle = new DLabel(this);
    headTitle->setFixedHeight(labelHeight);
    headTitle->setText(itemName);

    content->addWidget(headTitle);
    panel->setContentsMargins(0, 0, 0, 30);
    content->addWidget(panel);

    return true;
}

void OptionsDialog::slotLeftBarClicked(const QModelIndex &index)
{
    QString itemName = index.data().toString();

    auto y = widgets[itemName]->y() - labelHeight;
    scrollArea->verticalScrollBar()->setValue(y);
}

void OptionsDialog::setupUi(QWidget *widget)
{
    DFrame *mainFrame = new DFrame(widget);
    mainFrame->resize(910, 640);
    //titlebar
    DTitlebar *titlebar = new DTitlebar(mainFrame);
    titlebar->setMenuVisible(false);
    titlebar->setTitle(tr("Global Options"));

    QVBoxLayout *title = new QVBoxLayout(mainFrame);
    title->setContentsMargins(0, 0, 0, 0);
    title->setSpacing(0);

    // Center layout.
    auto mainLayout = new QHBoxLayout(mainFrame);
    mainLayout->setSpacing(6);
    mainLayout->setContentsMargins(11, 11, 11, 11);

    // Left layout.
    auto leftLayout = new QVBoxLayout(mainFrame);
    leftLayout->setSpacing(6);

    leftSideBar = new DListView(mainFrame);
    leftSideBar->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leftSideBar->setBackgroundRole(QPalette::ColorRole::Light);
    leftBarModel = new QStringListModel(leftSideBar);
    leftSideBar->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    QObject::connect(leftSideBar, &DListView::clicked, this, [=](const QModelIndex &index){
        activeOptName = index.data(Qt::DisplayRole).toString();
    });
    leftSideBar->setModel(leftBarModel);
    connect(leftSideBar, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(slotLeftBarClicked(const QModelIndex &)));

    leftLayout->addWidget(leftSideBar);

    // Right layout.
    auto rightLayout = new QVBoxLayout(mainFrame);
    rightLayout->setSpacing(6);

    //scollArea
    scrollArea = new QScrollArea(mainFrame);
    scrollWidget = new QWidget(mainFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollWidget);
    content = new QVBoxLayout(scrollWidget);
    rightLayout->addWidget(scrollArea);

    connect(scrollArea->verticalScrollBar(),&QScrollBar::valueChanged,this,&OptionsDialog::slotScrollChanged);

    auto buttonLayout = new QHBoxLayout(mainFrame);
    buttonLayout->setSpacing(6);
    auto horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    buttonLayout->addItem(horizontalSpacer);

    auto okBtn = new DPushButton(tr("OK"), mainFrame);
    connect(okBtn, SIGNAL(clicked()), this, SLOT(saveAllConfig()));

    auto cancelBtn = new DPushButton(tr("Cancel"), mainFrame);
    connect(cancelBtn, &DPushButton::clicked, [this] {
        // TODO(Mozart)
        this->close();
    });
    auto applyBtn = new DPushButton(tr("Apply"), mainFrame);
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

    title->addWidget(titlebar);
    title->addLayout(mainLayout);
}

void OptionsDialog::saveAllConfig()
{
    for (int index = 0; index < widgets.count(); index++)
    {
        auto key = widgets.keys().at(index);
        PageWidget* widget = widgets.value(key);
        if (widget) {
            widget->saveConfig();
        }
    }

    accept();
}


//TODO(zta): 之前是stackedWidget情况下，对单个页面修改后，点击apply进行单个页面的配置保存
//      现在右侧变为scrollArea，需要判断是哪个页面进行了修改。
void OptionsDialog::saveSingleConfig()
{
    for (int index = 0; index < widgets.count(); index++)
    {
        auto key = widgets.keys().at(index);
        PageWidget* widget = widgets.value(key);
        if (widget) {
            widget->saveConfig();
        }
    }
}

void OptionsDialog::readConfig()
{
    for (int index = 0; index < widgets.count(); index++)
    {
        auto key = widgets.keys().at(index);
        PageWidget* widget = widgets.value(key);
        if (widget) {
            widget->readConfig();
        }
    }
}

void OptionsDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    // Set leftbar item seleted.
    auto allOptName = leftBarModel->stringList();
    if (activeOptName.isEmpty() && allOptName.size() > 0){
        activeOptName = allOptName[0];
    }

    int index = allOptName.indexOf(activeOptName);
    if (index >= 0) {
        leftSideBar->selectionModel()->select(leftBarModel->index(index), QItemSelectionModel::Select);
    }

    readConfig();
}

void OptionsDialog::slotScrollChanged(int value)
{
    if (value == scrollArea->verticalScrollBar()->maximum())
        return;

    leftSideBar->clearSelection();

    for (auto row = 0; row < leftBarModel->rowCount() - 1; row++) {
        auto index = leftBarModel->index(row);
        auto widget = widgets.value(index.data().toString());
        if (widget && (value <= widget->y() - labelHeight)) {
            leftSideBar->selectionModel()->select(index, QItemSelectionModel::Select);
            activeOptName = index.data(Qt::DisplayRole).toString();
            break;
        }
    }
}
