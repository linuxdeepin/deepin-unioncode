/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "eventfilterdialog.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QTreeWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTreeWidget>
#include <QDebug>

namespace ReverseDebugger {
namespace Internal {

#define ARRAYSIZE(a) static_cast<int>(sizeof(a) / sizeof(a[0]))

// see `kill -l`
static const char *signalNames[] = {
    nullptr,   //0
    nullptr,   //1
    "SIGINT",   //2
    "SIGQUIT",   //3
    "SIGILL",   //4
    "SIGTRAP",   //5
    "SIGABRT",   //6
    "SIGBUS",   //7
    "SIGFPE",   //8
    nullptr,   //9
    nullptr,   //10
    "SIGSEGV",   //11
    nullptr,   //12
    "SIGPIPE",   //13
    nullptr,   //14
    "SIGTERM",   //15
    nullptr,   //16
    "SIGCHLD",   //17
    "SIGCONT",   //18
};

static const char *dbusNames[] = {
    nullptr,   //0
    "method call",   //1
    "method return",   //2
    "method error",   //3
    "send signal",   //4
};

static const char *x11Names[] = {
    nullptr,   //0
    nullptr,   //1
    "KeyPress",   //2
    "KeyRelease",   //3
    "ButtonPress",   //4
    "ButtonRelease",   //5
    nullptr,   //6
    nullptr,   //7
    nullptr,   //8
    "FocusIn",   //9
    "FocusOut",   //10
    nullptr,   //11
    nullptr,   //12
    nullptr,   //13
    nullptr,   //14
    nullptr,   //15
    "CreateNotify",   //16
    "DestroyNotify",   //17
    "UnmapNotify",   //18
    "MapNotify",   //19
};

const char *syscallNames[] = {
    "desc",   //0
    "file",   //1
    "memory",   //2
    "process",   //3
    "signal",   //4
    "ipc",   //5
    "network",   //6
    "vdso",   //7
};

static const char *syscallKindTips[] = {
    "Trace all file descriptor related system calls.",
    "Trace all system calls which take a file name as an argument.",
    "Trace all memory mapping related system calls.",
    "Trace all system calls which involve process management.",
    "Trace all signal related system calls.",
    "Trace all IPC related system calls.",
    "Trace all the network related system calls.",
    "Trace some time function in vdso:clock_gettime, gettimeofday, time.",
};

#define SIGNAL_BASE 100
#define X11_BASE 200
#define DBUS_BASE 300

class EventFilterDialogPrivate
{
public:
    QLineEdit *breakFunc = nullptr;
    QLineEdit *globalVar = nullptr;
    QLineEdit *maxHeapSize = nullptr;
    QLineEdit *maxStackSize = nullptr;
    QLineEdit *maxParamSize = nullptr;
    QCheckBox *onlyCurrentThread = nullptr;
    QTreeWidget *treeWidget = nullptr;
    QDialogButtonBox *buttonBox = nullptr;

    uchar *syscallFlags = nullptr;
    uchar *dbusFlags = nullptr;
    uchar *x11Flags = nullptr;
    uchar *signalFlags = nullptr;
};

EventFilterDialog::EventFilterDialog(
        QWidget *parent,
        uchar *syscallFlags,
        uchar *dbusFlags,
        uchar *x11Flags,
        uchar *signalFlags)
    : QDialog(parent),
      d(new EventFilterDialogPrivate)
{
    d->syscallFlags = syscallFlags;
    d->dbusFlags = dbusFlags;
    d->x11Flags = x11Flags;
    d->signalFlags = signalFlags;

    setupUi();
}

ReverseDebugger::Internal::EventFilterDialog::~EventFilterDialog()
{
    delete d;
}

int ReverseDebugger::Internal::EventFilterDialog::exec()
{
    connect(d->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(d->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(d->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this, SLOT(itemClicked(QTreeWidgetItem *, int)));

    d->treeWidget->expandAll();

    return QDialog::exec();
}

QString EventFilterDialog::syscallKindNames() const
{
    QString ret;
    // the last is vdso
    for (int i = 0; i < ARRAYSIZE(syscallNames) - 1; ++i) {
        if (d->syscallFlags[i] != 0) {
            ret += QLatin1String(syscallNames[i]);
            ret += QLatin1Char(',');
        }
    }

    return ret;
}

QString EventFilterDialog::breakFunc() const
{
    return d->breakFunc->text();
}

QString EventFilterDialog::globalVar() const
{
    return d->globalVar->text();
}

QString EventFilterDialog::maxStackSize() const
{
    return d->maxStackSize->text().trimmed();
}

QString EventFilterDialog::maxHeapSize() const
{
    return d->maxHeapSize->text().trimmed();
}

QString EventFilterDialog::maxParamSize() const
{
    return d->maxParamSize->text().trimmed();
}

bool EventFilterDialog::onlyCurrentThread() const
{
    return d->onlyCurrentThread->isChecked();
}

void EventFilterDialog::setMaxStackSize(const QString &size)
{
    d->maxStackSize->setText(size);
}

void EventFilterDialog::setMaxHeapSize(const QString &size)
{
    d->maxHeapSize->setText(size);
}

void EventFilterDialog::setMaxParamSize(const QString &size)
{
    d->maxParamSize->setText(size);
}

void EventFilterDialog::setOnlyCurrentThread(bool b)
{
    d->onlyCurrentThread->setChecked(b);
}

void EventFilterDialog::itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)

    int index = item->data(0, Qt::UserRole).toInt();
    int state = item->checkState(0);
    // Unchecked, PartiallyChecked, Checked

    if (index < SIGNAL_BASE) {
        d->syscallFlags[index] = (state == Qt::Checked);
        qDebug() << "click tree item" << index
                 << ":" << syscallNames[index] << ", state=" << state;
    } else if (index < X11_BASE) {
        index -= SIGNAL_BASE;
        d->signalFlags[index] = (state == Qt::Checked);
        qDebug() << "click tree item" << index
                 << ":" << signalNames[index] << ", state=" << state;
    } else if (index < DBUS_BASE) {
        index -= X11_BASE;
        d->x11Flags[index] = (state == Qt::Checked);
        qDebug() << "click tree item" << index
                 << ":" << x11Names[index] << ", state=" << state;
    } else {
        index -= DBUS_BASE;
        d->dbusFlags[index] = (state == Qt::Checked);
        qDebug() << "click tree item" << index
                 << ":" << dbusNames[index] << ", state=" << state;
    }
}

void EventFilterDialog::setupUi()
{
    setWindowTitle(tr("event filter."));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    d->buttonBox = new QDialogButtonBox(this);
    d->buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    d->buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

    d->treeWidget = new QTreeWidget(this);
    d->treeWidget->setColumnCount(1);

    if (QTreeWidgetItem *header = d->treeWidget->headerItem()) {
        header->setText(0, tr("event list"));
    } else {
        d->treeWidget->setHeaderLabel(tr("event list"));
    }

    // system call check list.
    QTreeWidgetItem *syscall = new QTreeWidgetItem(
            (QTreeWidget *)0, QStringList(tr("syscall filter")));
    for (int i = 0; i < ARRAYSIZE(syscallNames); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(syscall,
                                                    QStringList(tr(syscallNames[i])));
        item->setData(0, Qt::UserRole, i);
        item->setCheckState(0,
                            (0 == d->syscallFlags[i]) ? Qt::Unchecked : Qt::Checked);
        item->setToolTip(0, tr(syscallKindTips[i]));

        syscall->addChild(item);
    }

    // x11 event check list.
    QTreeWidgetItem *x11events = new QTreeWidgetItem(
            (QTreeWidget *)0, QStringList(tr("x11 events filter")));
    for (int i = 0; i < ARRAYSIZE(x11Names); ++i) {
        if (nullptr == x11Names[i]) {
            d->x11Flags[i] = 0;
            continue;
        }

        QTreeWidgetItem *item = new QTreeWidgetItem(x11events,
                                                    QStringList(tr(x11Names[i])));
        item->setData(0, Qt::UserRole, i + X11_BASE);
        item->setCheckState(0,
                            (0 == d->x11Flags[i]) ? Qt::Unchecked : Qt::Checked);

        x11events->addChild(item);
    }

    // dbus message event list.
    QTreeWidgetItem *dbusmsg = new QTreeWidgetItem(
            (QTreeWidget *)0, QStringList(tr("dbus filter")));
    for (int i = 0; i < ARRAYSIZE(dbusNames); ++i) {
        if (nullptr == dbusNames[i]) {
            d->dbusFlags[i] = 0;
            continue;
        }

        QTreeWidgetItem *item = new QTreeWidgetItem(dbusmsg,
                                                    QStringList(tr(dbusNames[i])));
        item->setData(0, Qt::UserRole, i + DBUS_BASE);
        item->setCheckState(0,
                            (0 == d->dbusFlags[i]) ? Qt::Unchecked : Qt::Checked);

        dbusmsg->addChild(item);
    }

    // insert three type of item to top level.
    d->treeWidget->insertTopLevelItems(0, { syscall, x11events, dbusmsg });

    d->maxHeapSize = new QLineEdit(this);
    d->maxStackSize = new QLineEdit(this);
    d->maxParamSize = new QLineEdit(this);
    d->onlyCurrentThread = new QCheckBox(this);
    d->globalVar = new QLineEdit(this);
    d->breakFunc = new QLineEdit(this);

    auto verticalLayout = new QVBoxLayout(this);
    verticalLayout->addWidget(d->treeWidget);
    verticalLayout->addStretch();

    auto formLayout = new QFormLayout();
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    formLayout->addRow(tr("Only record the thread where event occurred:"), d->onlyCurrentThread);
    formLayout->addRow(tr("Record the size of heap memory (default is 0, in KB):"), d->maxHeapSize);
    formLayout->addRow(tr("Record the size of stack memory (the default is 32, in KB):"), d->maxStackSize);
    formLayout->addRow(tr("Record system call parameter size (default is 256, unit Byte):"), d->maxParamSize);
    formLayout->addRow(tr("Record the specified global variable (format: [*]var1+size1[,[*]var2+size2,...]):"), d->globalVar);
    formLayout->addRow(tr("Start record after the specified function is executed (c++ mangle name):"), d->breakFunc);

    verticalLayout->addLayout(formLayout);
    verticalLayout->addStretch();
    verticalLayout->addWidget(d->buttonBox);
}

}   // namespace Internal
}   // namespace Debugger
