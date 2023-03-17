/*
 * Copyright (C) 2020 ~ 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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
#include "valgrindbar.h"
#include "valgrindrunner.h"
#include "xmlstreamreader.h"

#include <QTableWidget>
#include <QTreeWidget>
#include <QHeaderView>
#include <QHBoxLayout>

static QStringList memcheckItemNames {QObject::tr("Issue"), QObject::tr("Location")};
static QStringList helgrindItemNames {QObject::tr("Issue"), QObject::tr("Location")};

class ValgrindBarPrivate
{
    friend class ValgrindBar;
    QTreeWidget *memcheckWidget {nullptr};
    QTreeWidget *helgrindWidget {nullptr};
    QTabWidget *tabWidget {nullptr};
};

ValgrindBar::ValgrindBar(QWidget *parent)
    : QWidget(parent)
    , d(new ValgrindBarPrivate())
{
    d->tabWidget = new QTabWidget(this);
    d->memcheckWidget = new QTreeWidget(d->tabWidget);
    d->helgrindWidget = new QTreeWidget(d->tabWidget);

    initValgrindbar();

    QObject::connect(ValgrindRunner::instance(), &ValgrindRunner::valgrindFinished, this, &ValgrindBar::showResult);
    QObject::connect(ValgrindRunner::instance(), &ValgrindRunner::clearValgrindBar, this, &ValgrindBar::clearDisplay);
}

void ValgrindBar::refreshDisplay(QTreeWidget *treeWidget)
{
    treeWidget->clear();
}

void ValgrindBar::clearDisplay(const QString &type)
{
    if ("memcheck" == type) {
        d->memcheckWidget->clear();
    } else if ("helgrind" == type) {
        d->helgrindWidget->clear();
    }
}

void ValgrindBar::initValgrindbar()
{
    setWidgetStyle(d->memcheckWidget, memcheckItemNames);
    setWidgetStyle(d->helgrindWidget, helgrindItemNames);

    d->tabWidget->addTab(d->memcheckWidget, tr("memcheck"));
    d->tabWidget->addTab(d->helgrindWidget, tr("helgrind"));
    d->tabWidget->setTabPosition(QTabWidget::South);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    this->setLayout(hLayout);
    hLayout->addWidget(d->tabWidget);
}

void ValgrindBar::setWidgetStyle(QTreeWidget *treeWidget, const QStringList &itemNames)
{
    treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    treeWidget->setHeaderLabels(itemNames);
    treeWidget->setColumnCount(itemNames.size());
}

void ValgrindBar::showResult(const QString &xmlFileName, const QString &type)
{
    QTreeWidget *treeWidget = nullptr;

    if ("memcheck" == type) {
        treeWidget = d->memcheckWidget;
    } else if ("helgrind" == type){
        treeWidget = d->helgrindWidget;
    }

    d->tabWidget->setCurrentWidget(treeWidget);
    refreshDisplay(treeWidget);

    XmlStreamReader reader(treeWidget);
    reader.readFile(xmlFileName);
    editor.switchContext(tr("&Valgrind"));
}

