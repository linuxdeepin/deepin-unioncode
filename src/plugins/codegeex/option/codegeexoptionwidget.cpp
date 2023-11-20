// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeexoptionwidget.h"
#include "detailwidget.h"

#include "services/option/optionutils.h"
#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <DTabWidget>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

static const char *kCATEGORY_CODEGEEX = "CodeGeeX";

class CodeGeeXOptionWidgetPrivate
{
    DTabWidget* tabWidget = nullptr;
    friend class CodeGeeXOptionWidget;
};

CodeGeeXOptionWidget::CodeGeeXOptionWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new CodeGeeXOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout();
    d->tabWidget = new DTabWidget();
    d->tabWidget->tabBar()->setAutoHide(true);
    layout->addWidget(d->tabWidget);

    d->tabWidget->addTab(new DetailWidget(), tr("CodeGeeX"));
    QObject::connect(d->tabWidget, &DTabWidget::currentChanged, [this]() {
        readConfig();
    });

    setLayout(layout);
}

CodeGeeXOptionWidget::~CodeGeeXOptionWidget()
{
    if (d)
        delete d;
}

void CodeGeeXOptionWidget::saveConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            pageWidget->getUserConfig(map);
            OptionUtils::writeJsonSection(OptionUtils::getJsonFilePath(), kCATEGORY_CODEGEEX, itemNode, map);

            OptionManager::getInstance()->updateData();
        }
    }
}

void CodeGeeXOptionWidget::readConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            OptionUtils::readJsonSection(OptionUtils::getJsonFilePath(),
                                         kCATEGORY_CODEGEEX, itemNode, map);
            pageWidget->setUserConfig(map);
        }
    }
}

