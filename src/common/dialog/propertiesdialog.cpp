// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "propertiesdialog.h"
#include "common/widget/pagewidget.h"

#include <DPushButton>
#include <DTitlebar>
#include <DFrame>
#include <DBackgroundGroup>
#include <DSuggestButton>
#include <DTabWidget>

#include <QtDebug>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QSpacerItem>
#include <QStringListModel>

PropertiesDialog::PropertiesDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    setModal(true);
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

void PropertiesDialog::setupUi(DAbstractDialog *Dialog)
{
    auto vLayout = new QVBoxLayout(Dialog);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    DFrame *contentFrame = new DFrame();
    contentFrame->setLineWidth(0);

    auto contentlayout = new QHBoxLayout(contentFrame);
    contentlayout->setContentsMargins(10, 0, 0, 0);

    DTitlebar *titleBar = new DTitlebar(Dialog);
    titleBar->setMinimumHeight(43);
    titleBar->setMenuVisible(false);
    titleBar->setTitle(tr("Project Properties"));
    titleBar->setIcon(QIcon::fromTheme("unioncode"));
    vLayout->addWidget(titleBar);
    vLayout->addWidget(contentFrame);

    // Left layout.
    auto leftLayout = new QVBoxLayout();
    leftLayout->setContentsMargins(0, 10, 10, 0);
    leftLayout->setAlignment(Qt::AlignTop);
    leftSideBar = new DListView(Dialog);
    leftSideBar->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leftBarModel = new QStringListModel(leftSideBar);
    leftSideBar->setModel(leftBarModel);
    leftSideBar->setMaximumWidth(144);

    connect(leftSideBar, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(slotLeftBarClicked(const QModelIndex &)));

    connect(leftBarModel, &QStringListModel::modelReset, this, [=]{
        leftSideBar->setCurrentIndex(leftBarModel->index(0));
    });

    leftLayout->addWidget(leftSideBar);

    // Right layout.
    auto rightLayout = new QVBoxLayout();

    stackWidget = new DStackedWidget(Dialog);
    rightLayout->addWidget(stackWidget);

    QWidget *box = new QWidget();
    QHBoxLayout *box_layout = new QHBoxLayout(box);
    box_layout->setContentsMargins(0, 30, 0, 30);

    auto cancelBtn = new DPushButton(tr("Cancel"), box);
    connect(cancelBtn, &DPushButton::clicked, [this] {
        // TODO(Mozart)
        this->close();
    });
    auto applyBtn = new DSuggestButton(tr("Apply"), box);
    connect(applyBtn, SIGNAL(clicked()), this, SLOT(saveSingleConfig()));
    cancelBtn->setMinimumWidth(173);
    cancelBtn->setAutoDefault(false);
    applyBtn->setMinimumWidth(173);
    applyBtn->setAutoDefault(true);
    box_layout->setAlignment(Qt::AlignCenter);
    box_layout->addWidget(cancelBtn);
    box_layout->addSpacing(20);
    box_layout->addWidget(applyBtn);
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    rightLayout->addWidget(box);

    // Insert left & right layout to main layout.

    QVBoxLayout *bgGpLayout = new QVBoxLayout;
    bgGpLayout->setContentsMargins(0, 0, 0, 0);
    DBackgroundGroup *bgGroup = new DBackgroundGroup(bgGpLayout);
    bgGroup->setItemMargins(QMargins(0, 0, 0, 0));
    bgGroup->setBackgroundRole(QPalette::Window);
    bgGroup->setUseWidgetBackground(false);

    QWidget *wrapperWidget = new QWidget(bgGroup);
    QHBoxLayout *hLay = new QHBoxLayout(wrapperWidget);
    bgGpLayout->addWidget(wrapperWidget);
    hLay->addLayout(rightLayout);

    DStyle::setFrameRadius(bgGroup, 0);
    DStyle::setFrameRadius(contentFrame, 0);

    contentlayout->addLayout(leftLayout);
    contentlayout->addWidget(bgGroup);

    Dialog->setFixedSize(QSize(828, 800));
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
