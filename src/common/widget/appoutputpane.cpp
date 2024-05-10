// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appoutputpane.h"

#include <DTabBar>
#include <DDialog>
#include <DStackedWidget>
#include <DPalette>
#include <DComboBox>
#include <DToolButton>
#include <DStyle>

#include <QVBoxLayout>
#include <QProcess>
#include <QFileInfo>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

void killProcess(const quint64 pid)
{
    QProcess::startDetached("kill -9 " + QString::number(pid));
}

class OutputWindowPrivate
{
    friend class AppOutputPane;

public:
    OutputPane *defaultPane { nullptr };
    DWidget *tabbar { nullptr };
    DFrame *hLine { nullptr };
    DComboBox *tabChosser { nullptr };
    DToolButton *closeProcessBtn { nullptr };
    DToolButton *closePaneBtn { nullptr };

    DStackedWidget *stackWidget { nullptr };
    QMap<quint64, OutputPane *> appPane; // pid-pane
    QMap<quint64, bool> appIsRunning;

    DDialog *checkCloseDialog { nullptr };
};

AppOutputPane::AppOutputPane(QWidget *parent)
    :DFrame(parent), d(new OutputWindowPrivate)
{
    initUi();
}

AppOutputPane::~AppOutputPane()
{
    if (d)
        delete d;
}

AppOutputPane *AppOutputPane::instance()
{
    static AppOutputPane *ins = new AppOutputPane;
    return ins;
}

OutputPane *AppOutputPane::defaultPane()
{
    return d->defaultPane;
}

void AppOutputPane::initUi()
{
    d->defaultPane = OutputPane::instance();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    this->setLineWidth(0);
    DStyle::setFrameRadius(this, 0);

    initTabWidget();
    d->stackWidget = new DStackedWidget(this);
    d->tabChosser->addItem(tr("default"));
    d->stackWidget->addWidget(d->defaultPane);
    d->stackWidget->setContentsMargins(10, 0, 0, 10);
    
    d->hLine = new DFrame(this);
    d->hLine->setFrameShape(QFrame::HLine);
    d->hLine->hide();

    mainLayout->addWidget(d->tabbar);
    mainLayout->addWidget(d->hLine);
    mainLayout->addWidget(d->stackWidget);
}

void AppOutputPane::initTabWidget()
{
    d->tabbar = new DWidget(this);
    d->tabbar->setFixedHeight(38);
    d->tabbar->setAutoFillBackground(true);
    d->tabbar->setBackgroundRole(DPalette::Base);
    d->tabbar->setContentsMargins(10, 2, 0, 2);

    QHBoxLayout *tabLayout = new QHBoxLayout(d->tabbar);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setAlignment(Qt::AlignLeft);

    d->tabChosser = new DComboBox(d->tabbar);
    d->tabChosser->setFixedSize(120, 28);

    d->closeProcessBtn = new DToolButton(d->tabbar);
    d->closeProcessBtn->setIcon(QIcon::fromTheme("common_stop"));
    d->closeProcessBtn->setEnabled(false);

    d->closePaneBtn = new DToolButton(d->tabbar);
    d->closePaneBtn->setIcon(QIcon::fromTheme("error"));
    d->closePaneBtn->setEnabled(false);

    tabLayout->addWidget(d->tabChosser);
    tabLayout->addWidget(d->closeProcessBtn);
    tabLayout->addWidget(d->closePaneBtn);
    d->tabbar->hide();

    connect(d->tabChosser, QOverload<int>::of(&DComboBox::currentIndexChanged), this, [=](int index){
        d->stackWidget->setCurrentIndex(index);
        auto pane = qobject_cast<OutputPane *>(d->stackWidget->currentWidget());
        auto pid = d->appPane.key(pane);

        d->closePaneBtn->setEnabled(index == 0 ? false : true);

        if (d->appIsRunning.contains(pid) && d->appIsRunning[pid] == true)
            d->closeProcessBtn->setEnabled(true);
        else
            d->closeProcessBtn->setEnabled(false);

    });
    connect(d->closeProcessBtn, &DToolButton::clicked, this, [=](){
        auto pane = qobject_cast<OutputPane *>(d->stackWidget->currentWidget());
        auto pid = d->appPane.key(pane);
        killProcess(pid);
        d->closeProcessBtn->setEnabled(false);
    });
    connect(d->closePaneBtn, &DToolButton::clicked, this, &AppOutputPane::slotCloseOutputPane);
}

OutputPane *AppOutputPane::getOutputPaneByPid(const quint64 &pid)
{
    if (d->appPane.contains(pid))
        return d->appPane[pid];

    return d->defaultPane;
}

void AppOutputPane::createApplicationPane(const quint64 &pid, const QString &program)
{
    d->tabbar->show();
    d->hLine->show();

    //check if exist avaliable pane of this name
    for (auto index = 0; index < d->stackWidget->count(); index++) {
        auto pane = qobject_cast<OutputPane *>(d->stackWidget->widget(index));
        if( pane->property("program") == program) {
            auto panePid = d->appPane.key(pane);
            if(d->appIsRunning.contains(panePid) && !d->appIsRunning[panePid]) {
                d->appIsRunning.insert(pid, true);
                d->appPane.remove(panePid);
                d->appPane.insert(pid, pane);
                d->tabChosser->setCurrentIndex(index);
                d->closeProcessBtn->setEnabled(true);
                return;
            }
        }
    }

    auto pane = new OutputPane(this);
    pane->setProperty("program", program);
    d->appPane.insert(pid, pane);
    d->appIsRunning.insert(pid, true);

    auto file = QFileInfo(program);
    if (file.exists())
        d->tabChosser->addItem(file.baseName());
    else
        d->tabChosser->addItem(program);

    d->stackWidget->addWidget(pane);
    d->tabChosser->setCurrentIndex(d->tabChosser->count() - 1);

    emit paneCreated(pid);
}

void AppOutputPane::setProcessFinished(const quint64 &pid)
{
    if (!d->appIsRunning.contains(pid))
        return;
    d->appIsRunning[pid] = false;
    if (d->stackWidget->currentWidget() == d->appPane[pid])
        d->closeProcessBtn->setEnabled(false);
}

void AppOutputPane::appendTextToApplication(const quint64 &pid, const QString &text, OutputPane::OutputFormat format, OutputPane::AppendMode mode){
    if (d->appPane.contains(pid))
        d->appPane[pid]->appendText(text, format, mode);
    else
        d->defaultPane->appendText(text, format, mode);
}

void AppOutputPane::slotCloseOutputPane()
{
    auto index = d->tabChosser->currentIndex();
    auto *pane = qobject_cast<OutputPane *>(d->stackWidget->widget(index));
    if (pane && d->appPane.values().contains(pane)) {
        auto pid = d->appPane.key(pane);
        //check is running
        if (d->appIsRunning.contains(pid) && d->appIsRunning[pid] == true) {
            d->checkCloseDialog = new DDialog(this);
            d->checkCloseDialog->setIcon(QIcon::fromTheme("dialog-warning"));
            d->checkCloseDialog->setMessage(tr("Process is running, kill process?"));
            d->checkCloseDialog->insertButton(0, tr("kill", "button"), true, DDialog::ButtonWarning);
            d->checkCloseDialog->insertButton(1, tr("Cancel", "button"));

            connect(d->checkCloseDialog, &DDialog::buttonClicked, this, [=](int buttonIndex){
                if (buttonIndex == 0) {
                    killProcess(pid);
                    d->stackWidget->removeWidget(pane);
                    d->tabChosser->removeItem(index);
                    d->appPane.remove(pid);
                    d->appIsRunning.remove(pid);
                    delete pane;
                } else if (buttonIndex == 1) {
                    d->checkCloseDialog->reject();
                }
            });
            d->checkCloseDialog->setAttribute(Qt::WA_DeleteOnClose);
            d->checkCloseDialog->exec();
        } else {
            d->tabChosser->removeItem(index);
            d->stackWidget->removeWidget(pane);
        }
    }

    if (d->tabChosser->count() == 1 && d->stackWidget->currentWidget() == d->defaultPane)
        d->tabbar->hide();
}
