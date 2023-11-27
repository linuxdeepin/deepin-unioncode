// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmakeoptionwidget.h"
#include "kitsmanagerwidget.h"

#include "services/option/optionutils.h"
#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <QHBoxLayout>
#include <DTabWidget>

class CMakeOptionWidgetPrivate {
    DTabWidget* tabWidget = nullptr;

    friend class CMakeOptionWidget;
};

CMakeOptionWidget::CMakeOptionWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new CMakeOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout();
    d->tabWidget = new DTabWidget();
    d->tabWidget->tabBar()->setAutoHide(true);
    layout->addWidget(d->tabWidget);

    d->tabWidget->addTab(new KitsManagerWidget(), "Kits");
    //去除tabwidget的边框
    d->tabWidget->setDocumentMode(true);
    QObject::connect(d->tabWidget, &DTabWidget::currentChanged, [this]() {
        readConfig();
    });

    setLayout(layout);
}

CMakeOptionWidget::~CMakeOptionWidget()
{
    if (d)
        delete d;
}

void CMakeOptionWidget::saveConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++)
    {
        PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            pageWidget->getUserConfig(map);
            OptionUtils::writeJsonSection(OptionUtils::getJsonFilePath(),
                                          option::CATEGORY_CMAKE, itemNode, map);

            OptionManager::getInstance()->updateData();
        }
    }
}

void CMakeOptionWidget::readConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++)
    {
        PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            OptionUtils::readJsonSection(OptionUtils::getJsonFilePath(),
                                         option::CATEGORY_CMAKE, itemNode, map);
            pageWidget->setUserConfig(map);
        }
    }
}

