// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

