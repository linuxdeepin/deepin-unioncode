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
#include "reversedebuggermgr.h"
#include "reversedebuggerconstants.h"
#include "minidumpruncontrol.h"
#include "eventfilterdialog.h"
#include "event_man.h"
#include "taskwindow.h"
#include "taskmodel.h"
#include "taskfiltermodel.h"
#include "timelinewidget.h"
#include "loadcoredialog.h"
#include "services/debugger/debuggerservice.h"
#include "common/actionmanager/actionmanager.h"
#include "services/project/projectservice.h"
#include "services/window/windowservice.h"

#include <QtConcurrent>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QMessageBox>

#define AsynInvoke(Fun)          \
    QtConcurrent::run([this]() { \
        Fun;                     \
    });

extern bool g_emd_running;
void* g_timeline = nullptr;

// default flags count.
#define X11_FLAGS_COUNT        20
#define SIGNAL_FLAGS_COUNT     20
#define SYSCALL_FLAGS_COUNT    8
#define DBUS_FLAGS_COUNT       5

using namespace dpfservice;

namespace ReverseDebugger {
namespace Internal {

static TaskWindow* g_taskWindow = nullptr;

ReverseDebuggerMgr::ReverseDebuggerMgr(QObject *parent)
    : QObject(parent), runCtrl(new MinidumpRunControl(this))
{
    initialize();
}

void ReverseDebuggerMgr::initialize()
{
    if (!g_taskWindow) {
        g_taskWindow = new TaskWindow;

        g_taskWindow->addCategory(Constants::EVENT_CATEGORY_SYSCALL, tr("syscall"), true);
        g_taskWindow->addCategory(Constants::EVENT_CATEGORY_SIGNAL, tr("signal"), true);
        g_taskWindow->addCategory(Constants::EVENT_CATEGORY_X11, tr("x11"), true);
        g_taskWindow->addCategory(Constants::EVENT_CATEGORY_DBUS, tr("dbus"), true);
        connect(g_taskWindow, SIGNAL(coredumpChanged(int)),
                this, SLOT(runCoredump(int)));
        connect(g_taskWindow, SIGNAL(tasksCleared()),
                this, SLOT(unloadMinidump()));
    }
}

void ReverseDebuggerMgr::recored()
{
    recordMinidump();
}

void ReverseDebuggerMgr::replay()
{
    enterReplayEnvironment();

    QString defaultTraceDir = QDir::homePath() + QDir::separator() + ".local/share/emd/latest-trace";

    LoadCoreDialog dlg;
    CoredumpRunParameters parameters = dlg.displayDlg(defaultTraceDir);

    replayMinidump(parameters.tracedir, parameters.pid);
}

QWidget *ReverseDebuggerMgr::getWidget() const
{
    QWidget *widget = new QWidget();

    // verhicle
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    widget->setLayout(vLayout);
    QHBoxLayout *hLayout = new QHBoxLayout();
    for (auto it : g_taskWindow->toolBarWidgets()) {
        hLayout->addWidget(it);
    }

    QSpacerItem *spaceItem = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayout->addSpacerItem(spaceItem);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(g_taskWindow->outputWidget());

    return widget;
}

QString ReverseDebuggerMgr::generateFilePath(const QString &fileName, const QString &traceDir, int pid)
{
    QString ret = traceDir + (traceDir.back() == '/' ? "" : "/");
    ret += fileName;
    ret += QString::number(pid);

    return ret;
}

void ReverseDebuggerMgr::replayMinidump(const QString &traceDir, int pid)
{
    if (g_timeline) {
        g_taskWindow->updateTimeline(nullptr, 0);
        destroy_timeline(g_timeline);
        g_timeline = nullptr;
    }

    // core file.
    QString corefile = generateFilePath(CONTEXT_FILE_NAME, traceDir, pid);
    if (corefile.isEmpty()) {
        outputMessage("Context file is empty!");
        return;
    }

    // map file.
    QString mapFile = generateFilePath(MAP_FILE_NAME, traceDir, pid);
    int frameCount = create_timeline(mapFile.toLocal8Bit(),
            corefile.toLocal8Bit(), &g_timeline);
    if (frameCount < 1) {
        QMessageBox msgBox;
        msgBox.setText(tr("Not found valid event in context file!"));
        msgBox.exec();
        return;
    }

    const EventEntry* entry = get_event_pointer(g_timeline);
    if (entry) {
        // Add task.
        const char* cats[] = {
            ReverseDebugger::Constants::EVENT_CATEGORY_SYSCALL,
            ReverseDebugger::Constants::EVENT_CATEGORY_SIGNAL,
            ReverseDebugger::Constants::EVENT_CATEGORY_DBUS,
            ReverseDebugger::Constants::EVENT_CATEGORY_X11,
        };

        for (int i = 0; i < frameCount; ++i) {

            /* ON MIPS, there's these three value:
             22:#define __NR_Linux			4000
            406:#define __NR_Linux			5000
            749:#define __NR_Linux			6000
            */
            ReverseDebugger::Internal::Task task(
                      QString::asprintf("%d:%s", i, get_event_name(entry->type)),
                      cats[entry->type/1000 - __NR_Linux/1000],
                      entry);
            g_taskWindow->addTask(task);
            ++entry;
        }
    }
    g_taskWindow->updateTimeline(g_timeline, frameCount);

    // do something.
    // check if trace-dir/crash.txt exist? Auto load crash event if true.
    --entry;
    if (entry->type >= DUMP_REASON_signal && entry->type < DUMP_REASON_dbus) {
        g_taskWindow->goTo(frameCount - 1);
    }
}

void ReverseDebuggerMgr::outputMessage(const QString &msg)
{
    // TODO(mozart):display the message to outputpane.
    qDebug() << msg;
}

void ReverseDebuggerMgr::exist()
{
    auto command = ActionManager::getInstance()->command("Debug.Abort.Debugging");
    QAction *action = nullptr;
    if (command && (action = command->action()) && action->isEnabled()) {
        action->trigger();
    }
}

QString ReverseDebuggerMgr::targetPath() const
{
    QString targetPath;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService && projectService->getActiveTarget) {
        Target target = projectService->getActiveTarget(kActiveExecTarget);
        targetPath = target.outputPath + QDir::separator() + target.path +  QDir::separator() + target.buildTarget;
    }
    return targetPath;
}

void ReverseDebuggerMgr::enterReplayEnvironment()
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService && windowService->switchWidgetContext) {
        emit windowService->switchWidgetContext(tr("Reverse Debug"));
    }
}

static void NumberList2QString(uchar *in, int size, QString &str)
{
    str.clear();
    for (int i = 0; i < size; ++i) {
        if (in[i] != 0) {
            str += QString::number(i) + QLatin1Char(',');
        }
    }
}

static void ParseNumberList(QString &str, unsigned char *out, size_t size)
{
    QStringList ret = str.split(QLatin1Char(','));

    memset(out, 0, size);

    if (ret.size() == 0) {
        return;
    }

    QStringList::const_iterator it;
    for (it = ret.constBegin(); it != ret.constEnd(); ++it) {
        out[(*it).toInt()] = 1;
    }
}

void ReverseDebuggerMgr::recordMinidump()
{
    if (g_emd_running) {
        qDebug() << "emd is running";
        return;
    }

    uchar x11Flags[X11_FLAGS_COUNT] = {0};
    uchar signalFlags[SIGNAL_FLAGS_COUNT] = {0};
    uchar syscallFlags[SYSCALL_FLAGS_COUNT] = {0};
    uchar dbusFlags[DBUS_FLAGS_COUNT] = {0};

    QString syscall = configValue("SyscallFilter").toString();
    QString signal = configValue("SignalFilter").toString();
    QString x11 = configValue("X11Filter").toString();
    QString dbus = configValue("DbusFilter").toString();

    ParseNumberList(syscall, syscallFlags, sizeof(syscallFlags));
    ParseNumberList(signal, signalFlags, sizeof(signalFlags));
    ParseNumberList(x11, x11Flags, sizeof(x11Flags));
    ParseNumberList(dbus, dbusFlags, sizeof(dbusFlags));

    EventFilterDialog dlg(nullptr, syscallFlags, dbusFlags, x11Flags, signalFlags);

    QString maxStackSize = configValue("StackSize").toString();
    QString maxHeapSize = configValue("HeapSize").toString();
    QString maxParamSize = configValue("ParamSize").toString();
    bool onlyCurrentThread = configValue("CurrentThread").toBool();
    dlg.setMaxStackSize(maxStackSize);
    dlg.setMaxHeapSize(maxHeapSize);
    dlg.setMaxParamSize(maxParamSize);
    dlg.setOnlyCurrentThread(onlyCurrentThread);
    if (dlg.exec() != QDialog::Accepted)
        return;

    maxStackSize = dlg.maxStackSize();
    maxHeapSize = dlg.maxHeapSize();
    maxParamSize = dlg.maxParamSize();
    onlyCurrentThread = dlg.onlyCurrentThread();
    setConfigValue("StackSize", maxStackSize);
    setConfigValue("HeapSize", maxHeapSize);
    setConfigValue("ParamSize", maxParamSize);
    setConfigValue("CurrentThread", onlyCurrentThread);

    NumberList2QString(syscallFlags, sizeof(syscallFlags), syscall);
    NumberList2QString(signalFlags, sizeof(signalFlags), signal);
    NumberList2QString(x11Flags, sizeof(x11Flags), x11);
    NumberList2QString(dbusFlags, sizeof(dbusFlags), dbus);

    setConfigValue("SyscallFilter", syscall);
    setConfigValue("SignalFilter", signal);
    setConfigValue("X11Filter", x11);
    setConfigValue("DbusFilter", dbus);

    QString emdParams;

    //NOTE: There's no need to store global variable name and break function.
    // They are depend on the specified project.
    QString globalVar = dlg.globalVar();
    if (!globalVar.isEmpty()) {
        qDebug() << "global var:" << globalVar;
        emdParams += QLatin1String(" --var=") + globalVar;
    }

    QString breakFunc = dlg.breakFunc();
    if (!breakFunc.isEmpty()) {
        qDebug() << "break func:" << breakFunc;
        emdParams += QLatin1String(" --func=") + breakFunc;
    }

    if (syscallFlags[7]) {
        qDebug() << "hook vdso";
        emdParams += QLatin1String(" --vdso=on");
    }

    if (syscall.size()) {
        qDebug() << "syscall:" << syscall;
        emdParams += QLatin1String(" --sys=") + dlg.syscallKindNames();
    }
    if (x11.size()) {
        qDebug() << "x11:" << x11;
        emdParams += QLatin1String(" --x11=") + x11;
    }
    if (dbus.size()) {
        qDebug() << "dbus:" << syscall;
        emdParams += QLatin1String(" --dbus=") + dbus;
    }

    if (maxStackSize.size()) {
        emdParams += QLatin1String(" --stack-size=") + maxStackSize;
    }

    if (maxHeapSize.size()) {
        emdParams += QLatin1String(" --heap-size=") + maxHeapSize;
    }

    if (maxParamSize.size()) {
        emdParams += QLatin1String(" --param-size=") + maxParamSize;
    }

    if (onlyCurrentThread) {
        emdParams += QLatin1String(" -1");
    }

    if (runCtrl) {
        runCtrl->start(emdParams, targetPath());
    }
}

void ReverseDebuggerMgr::runCoredump(int index)
{
    qDebug() << Q_FUNC_INFO << ", " << index;

    // produce coredump.
    if (0 == generate_coredump(g_timeline, index, "/tmp/emd.core", 0)) {
        // run coredump.
        // gdb target core.
        auto &ctx = dpfInstance.serviceContext();
        auto service = ctx.service<DebuggerService>(DebuggerService::name());
        if (service) {
            if (service->runCoredump) {
                service->runCoredump("target", "core", "cmake");
            }
        }
    } else {
        qDebug() << "Failed to create coredump file:" << index;
    }
}

void ReverseDebuggerMgr::unloadMinidump()
{

}

QVariant ReverseDebuggerMgr::configValue(const QByteArray &name)
{
    // here are temporay value.
    QHash<QString, QString> values { { "StackSize", "32" },
                                     { "HeapSize", "0" },
                                     { "ParamSize", "256" },
                                     { "CurrentThread", "true" }};

    return values[name];
}

void ReverseDebuggerMgr::setConfigValue(const QByteArray &name, const QVariant &value)
{
    Q_UNUSED(name)
    Q_UNUSED(value)

    // TODO(mozart):persistent should be done.
}

}   // namespace Internal
}   // namespace ReverseDebugger
