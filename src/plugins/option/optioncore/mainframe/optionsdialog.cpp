// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionsdialog.h"
#include "common/widget/pagewidget.h"
#include "services/option/optionservice.h"
#include "navigationdelegate.h"

#include <DTitlebar>
#include <DFrame>
#include <DBackgroundGroup>
#include <DSuggestButton>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#include <DPaletteHelper>
#endif
#include <DGuiApplicationHelper>

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
    setupUi();
}

bool OptionsDialog::insertOptionPanel(const QString &itemName, PageWidget *panel)
{
    widgets.insert(itemName, panel);

    auto item = new QStandardItem;
    item->setData(itemName, Qt::DisplayRole);
    item->setData(NavigationDelegate::Level2, NavigationDelegate::NavLevelRole);

    leftBarModel->appendRow(item);

    DLabel *headTitle = new DLabel(this);
    headTitle->setFixedHeight(labelHeight);
    headTitle->setText(itemName);

    QVBoxLayout *bgGplayout = new QVBoxLayout;
    DBackgroundGroup *bgGroup = new DBackgroundGroup(bgGplayout);
    bgGroup->setFixedWidth(685);
    bgGroup->setContentsMargins(0, 0, 0, 30);
    bgGroup->setBackgroundRole(QPalette::Window);
    bgGroup->setUseWidgetBackground(false);

    bgGplayout->addWidget(panel);

    content->addWidget(headTitle);
    content->addWidget(bgGroup);

    itemList.insert(item, headTitle);

    return true;
}

bool OptionsDialog::insertLabel(const QString &itemName)
{
    auto item = new QStandardItem;
    item->setData(itemName, Qt::DisplayRole);
    item->setData(NavigationDelegate::Level1, NavigationDelegate::NavLevelRole);

    leftBarModel->appendRow(item);

    DLabel *group = new DLabel(this);
    auto font = group->font();
    font.setBold(true);
    font = DFontSizeManager::instance()->get(DFontSizeManager::T4, font);
    group->setFont(font);
    group->setFixedHeight(labelHeight);
    group->setText(itemName);
    content->addWidget(group);

    itemList.insert(item, group);

    return true;
}

void OptionsDialog::showAtItem(const QString &itemName)
{
    show();

    QModelIndex index = leftBarModel->indexFromItem(leftBarModel->findItems(itemName).at(0));
    slotLeftBarClicked(index);
}

void OptionsDialog::slotLeftBarClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    auto item = leftBarModel->itemFromIndex(index);
    auto y = itemList[item]->y();
    scrollArea->verticalScrollBar()->setValue(y);
}

void OptionsDialog::setupUi()
{
    resize(910, 640);

    //titlebar
    DTitlebar *titlebar = new DTitlebar(this);
    titlebar->setMenuVisible(false);
    titlebar->setTitle(tr("Global Options"));
    titlebar->setIcon(QIcon::fromTheme("ide"));

    QVBoxLayout *title = new QVBoxLayout(this);
    title->setContentsMargins(0, 0, 0, 0);
    title->setSpacing(0);

    // Center layout.
    auto mainLayout = new QHBoxLayout;
    mainLayout->setSpacing(6);

    // Left layout.
    auto leftFrame = new DFrame(this);
    auto leftLayout = new QVBoxLayout(leftFrame);
    leftLayout->setSpacing(6);

    leftSideBar = new DListView(leftFrame);
    leftSideBar->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leftSideBar->setBackgroundRole(QPalette::ColorRole::Light);
    leftSideBar->setItemDelegate(new NavigationDelegate(leftSideBar));

#ifdef DTKWIDGET_CLASS_DPaletteHelper
    DPalette pa = DPaletteHelper::instance()->palette(leftSideBar);
    pa.setBrush(DPalette::ItemBackground, Qt::transparent);
    DPaletteHelper::instance()->setPalette(leftSideBar, pa);
#else
    DPalette pa = leftSideBar->palette();
    pa.setBrush(DPalette::Window, Qt::transparent);
    leftSideBar->setPalette(pa);
#endif

    leftBarModel = new QStandardItemModel(leftSideBar);
    leftSideBar->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    leftSideBar->setModel(leftBarModel);
    connect(leftSideBar, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(slotLeftBarClicked(const QModelIndex &)));

    leftLayout->addWidget(leftSideBar);

    // Right layout.
    auto rightFrame = new DFrame(this);
    auto rightLayout = new QVBoxLayout(rightFrame);

    //scollArea
    scrollArea = new QScrollArea(rightFrame);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollWidget = new DWidget(rightFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollWidget);
    content = new QVBoxLayout(scrollWidget);
    rightLayout->addWidget(scrollArea);

    connect(scrollArea->verticalScrollBar(),&QScrollBar::valueChanged,this,&OptionsDialog::slotScrollChanged);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(6);
    auto horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    buttonLayout->addItem(horizontalSpacer);

    auto okBtn = new DSuggestButton(tr("OK"), this);
    connect(okBtn, SIGNAL(clicked()), this, SLOT(saveAllConfig()));

    auto cancelBtn = new DPushButton(tr("Cancel"), this);
    connect(cancelBtn, &DPushButton::clicked, [this] {
        // TODO(Mozart)
        this->close();
    });

    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);

    rightLayout->addLayout(buttonLayout);

    // Insert left & right layout to main layout.
    mainLayout->addWidget(leftFrame);
    mainLayout->addWidget(rightFrame);

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

    options.configSaved();
    accept();
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

    if (!currentIndex.isValid())
        leftSideBar->setCurrentIndex(leftBarModel->index(0, 0));

    readConfig();
}

void OptionsDialog::slotScrollChanged(int value)
{
    leftSideBar->clearSelection();

    if (value == scrollArea->verticalScrollBar()->maximum()) {
        leftSideBar->setCurrentIndex(leftBarModel->index(leftBarModel->rowCount() - 1, 0));
        return;
    }

    for (auto row = 0; row < leftBarModel->rowCount() - 1; row++) {
        auto label = itemList[leftBarModel->item(row)];

        if (value <= label->y()) {
            auto index = leftBarModel->index(row, 0);
            leftSideBar->setCurrentIndex(index);
            currentIndex = index;
            return;
        }
    }
}
