// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pythonoptionwidget.h"
#include "interpreterwidget.h"

#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <QHBoxLayout>
#include <DTabWidget>

class PythonOptionWidgetPrivate
{
    DTabWidget *tabWidget = nullptr;

    friend class PythonOptionWidget;
};

PythonOptionWidget::PythonOptionWidget(QWidget *parent)
    : PageWidget(parent), d(new PythonOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout();
    d->tabWidget = new DTabWidget();
    d->tabWidget->tabBar()->setAutoHide(true);
    d->tabWidget->setDocumentMode(true);
    layout->addWidget(d->tabWidget);

    d->tabWidget->addTab(new InterpreterWidget(), "Interpreter");
    QObject::connect(d->tabWidget, &DTabWidget::currentChanged, [this]() {
        readConfig();
    });

    setLayout(layout);
}

PythonOptionWidget::~PythonOptionWidget()
{
    if (d)
        delete d;
}

void PythonOptionWidget::saveConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *pageWidget = qobject_cast<PageWidget *>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            pageWidget->getUserConfig(map);
            OptionManager::getInstance()->setValue(option::CATEGORY_PYTHON, itemNode, map);
        }
    }
}

void PythonOptionWidget::readConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++) {
        PageWidget *pageWidget = qobject_cast<PageWidget *>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map = OptionManager::getInstance()->getValue(option::CATEGORY_PYTHON, itemNode).toMap();
            pageWidget->setUserConfig(map);
        }
    }
}
