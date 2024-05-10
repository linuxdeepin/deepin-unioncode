// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "valgrindbar.h"
#include "valgrindrunner.h"
#include "xmlstreamreader.h"

#include <DTreeWidget>
#include <DHeaderView>
#include <DStackedWidget>
#include <DButtonBox>
#include <DFrame>

#include <QHBoxLayout>

class ValgrindBarPrivate
{
    friend class ValgrindBar;
    DFrame *mainFrame {nullptr};
    DTreeWidget *memcheckWidget {nullptr};
    DTreeWidget *helgrindWidget {nullptr};
    QTabWidget *tabWidget {nullptr};
    DStackedWidget *stackedWidget {nullptr};
    DButtonBox *btnBox {nullptr};
    DButtonBoxButton *memcheckBtn {nullptr};
    DButtonBoxButton *helgrindBtn {nullptr};
};

ValgrindBar::ValgrindBar(QWidget *parent)
    : QWidget(parent)
    , d(new ValgrindBarPrivate())
{
    d->mainFrame = new DFrame(this);
    d->stackedWidget = new DStackedWidget(d->mainFrame);
    d->memcheckWidget = new DTreeWidget(d->stackedWidget);
    d->helgrindWidget = new DTreeWidget(d->stackedWidget);

    initValgrindbar();

    QObject::connect(ValgrindRunner::instance(), &ValgrindRunner::valgrindFinished, this, &ValgrindBar::showResult);
    QObject::connect(ValgrindRunner::instance(), &ValgrindRunner::clearValgrindBar, this, &ValgrindBar::clearDisplay);
}

ValgrindBar::~ValgrindBar()
{
    delete d;
}

void ValgrindBar::refreshDisplay(DTreeWidget *treeWidget)
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
    int memcheckIndex = d->stackedWidget->addWidget(d->memcheckWidget);
    int helgrindIndex = d->stackedWidget->addWidget(d->helgrindWidget);

    d->memcheckWidget->header()->hide();
    d->memcheckWidget->setLineWidth(0);

    d->helgrindWidget->header()->hide();
    d->helgrindWidget->setLineWidth(0);

    d->btnBox = new DButtonBox(d->mainFrame);
    d->btnBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->memcheckBtn = new DButtonBoxButton(tr("memcheck"));
    d->helgrindBtn = new DButtonBoxButton(tr("helgrind"));
    QList<DButtonBoxButton*> list;
    list.append(d->memcheckBtn);
    list.append(d->helgrindBtn);
    d->btnBox->setButtonList(list, true);
    d->memcheckBtn->setFixedWidth(93);
    d->helgrindBtn->setFixedWidth(93);
    connect(d->memcheckBtn, &DButtonBoxButton::clicked, [=]{
        d->stackedWidget->setCurrentIndex(memcheckIndex);
    });

    connect(d->helgrindBtn, &DButtonBoxButton::clicked, [=]{
        d->stackedWidget->setCurrentIndex(helgrindIndex);
    });

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(d->btnBox);
    hLayout->setAlignment(Qt::AlignLeft);
    hLayout->setMargin(0);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayout);
    vLayout->addWidget(d->stackedWidget);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setMargin(0);
    d->mainFrame->setLayout(vLayout);
    d->mainFrame->setLineWidth(0);
    vLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(d->mainFrame);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    DStyle::setFrameRadius(d->mainFrame, 0);
    this->setLayout(mainLayout);
}

void ValgrindBar::showResult(const QString &xmlFileName, const QString &type)
{
    DTreeWidget *treeWidget = nullptr;

    if ("memcheck" == type) {
        d->memcheckBtn->setChecked(true);
        treeWidget = d->memcheckWidget;
    } else if ("helgrind" == type){
        d->helgrindBtn->setChecked(true);
        treeWidget = d->helgrindWidget;
    }

    d->stackedWidget->setCurrentWidget(treeWidget);
    refreshDisplay(treeWidget);

    XmlStreamReader reader(treeWidget);
    reader.readFile(xmlFileName);
    uiController.switchContext(tr("&Valgrind"));
}
