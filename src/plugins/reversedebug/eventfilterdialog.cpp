// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventfilterdialog.h"

#include <DLineEdit>
#include <DCheckBox>
#include <DTreeWidget>
#include <DPushButton>
#include <DSuggestButton>
#include <DTitlebar>
#include <DLabel>
#include <DFrame>

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDebug>

DWIDGET_USE_NAMESPACE
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
#define DIALOGWIDTH 528
#define DIALOGHEIGHT 661

class EventFilterDialogPrivate
{
public:
    DLineEdit *breakFunc = nullptr;
    DLineEdit *globalVar = nullptr;
    DLineEdit *maxHeapSize = nullptr;
    DLineEdit *maxStackSize = nullptr;
    DLineEdit *maxParamSize = nullptr;
    DCheckBox *onlyCurrentThread = nullptr;
    DTreeWidget *treeWidget = nullptr;
    DSuggestButton *applyButton = nullptr;
    DPushButton *rejectButton = nullptr;

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
    : DDialog(parent),
      d(new EventFilterDialogPrivate)
{
    d->syscallFlags = syscallFlags;
    d->dbusFlags = dbusFlags;
    d->x11Flags = x11Flags;
    d->signalFlags = signalFlags;
    setWindowTitle(tr("Event Filter"));
    setIcon(QIcon::fromTheme("ide"));

    setupUi();
}

ReverseDebugger::Internal::EventFilterDialog::~EventFilterDialog()
{
    delete d;
}

int ReverseDebugger::Internal::EventFilterDialog::exec()
{
    connect(d->rejectButton, &DPushButton::clicked, this, &QDialog::reject);
    connect(d->applyButton, &DSuggestButton::clicked, this, &QDialog::accept);
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
    resize(DIALOGWIDTH, DIALOGHEIGHT);

    auto mainFrame = new QFrame(this);
    auto verticalLayout = new QVBoxLayout(mainFrame);
    this->addContent(mainFrame);
    mainFrame->setLayout(verticalLayout);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    DLabel *treeHeader = new DLabel(mainFrame);
    treeHeader->setText(tr("event list"));

    auto *treeFrame = new DFrame(this);
    d->treeWidget = new DTreeWidget(treeFrame);
    d->treeWidget->setColumnCount(1);
    d->treeWidget->setHeaderHidden(true);
    d->treeWidget->setFrameShape(DFrame::Shape::NoFrame);

    QVBoxLayout *treelayout = new QVBoxLayout(treeFrame);
    treelayout->addWidget(d->treeWidget);

    // system call check list.
    QTreeWidgetItem *syscall = new QTreeWidgetItem(
            (DTreeWidget *)0, QStringList(tr("syscall filter")));
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
            (DTreeWidget *)0, QStringList(tr("x11 events filter")));
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
            (DTreeWidget *)0, QStringList(tr("dbus filter")));
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

    d->maxHeapSize = new DLineEdit(mainFrame);
    d->maxHeapSize->setPlaceholderText(tr("default is 0, in KB"));
    d->maxStackSize = new DLineEdit(mainFrame);
    d->maxStackSize->setPlaceholderText(tr("default is 32, unit Byte"));
    d->maxParamSize = new DLineEdit(mainFrame);
    d->maxParamSize->setPlaceholderText(tr("default is 256, unit Byte"));
    d->onlyCurrentThread = new DCheckBox(mainFrame);
    d->globalVar = new DLineEdit(mainFrame);
    d->globalVar->setPlaceholderText(tr("format: [*]var1+size1[,[*]var2+size2,...]"));
    d->breakFunc = new DLineEdit(mainFrame);
    d->breakFunc->setPlaceholderText(tr("c++ mangle name"));

    verticalLayout->addWidget(treeHeader);
    verticalLayout->addWidget(treeFrame);

    auto checkLayout = new QHBoxLayout(mainFrame);
    checkLayout->addWidget(d->onlyCurrentThread);
    checkLayout->addWidget(new DLabel(tr("Only record the thread where event occurred:")));
    checkLayout->setAlignment(Qt::AlignLeft);
    checkLayout->setContentsMargins(0, 10, 0, 0);

    auto formLayout = new QFormLayout(mainFrame);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    formLayout->addRow(tr("Record the size of heap memory :"), d->maxHeapSize);
    formLayout->addRow(tr("Record the size of stack memory :"), d->maxStackSize);
    formLayout->addRow(tr("Record system call parameter size :"), d->maxParamSize);
    formLayout->addRow(tr("Record the specified global variable :"), d->globalVar);
    formLayout->addRow(tr("Start record after the specified function is executed :"), d->breakFunc);
    formLayout->setSpacing(10);

    verticalLayout->addLayout(checkLayout);
    verticalLayout->addLayout(formLayout);

    auto buttonLayout = new QHBoxLayout(mainFrame);
    setupButton(buttonLayout);
    verticalLayout->addLayout(buttonLayout);
}

void EventFilterDialog::setupButton(QHBoxLayout *buttonLayout)
{
    DPushButton *rejectButton = new DPushButton(tr("Cancel"));
    DSuggestButton *applyButton = new DSuggestButton(tr("OK"));
    rejectButton->setFixedWidth(173);
    applyButton->setFixedWidth(173);

    DVerticalLine *line = new DVerticalLine;
    line->setObjectName("VLine");
    line->setFixedHeight(30);

    buttonLayout->addWidget(rejectButton);
    buttonLayout->addWidget(line);
    buttonLayout->addWidget(applyButton);
    buttonLayout->setAlignment(Qt::AlignHCenter);
    buttonLayout->setContentsMargins(0, 10, 0, 0);

    connect(rejectButton, &DPushButton::clicked, this, &DDialog::reject);
    connect(applyButton, &DPushButton::clicked, this, &DDialog::accept);
}

}   // namespace Internal
}   // namespace Debugger
