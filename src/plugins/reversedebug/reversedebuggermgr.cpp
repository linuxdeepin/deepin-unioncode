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
#include "minidumpruncontrol.h"
#include "eventfilterdialog.h"

#include <QtConcurrent>

#define AsynInvoke(Fun)          \
    QtConcurrent::run([this]() { \
        Fun;                     \
    });

extern bool g_emd_running;

// default flags count.
#define X11_FLAGS_COUNT        20
#define SIGNAL_FLAGS_COUNT     20
#define SYSCALL_FLAGS_COUNT    8
#define DBUS_FLAGS_COUNT       5

namespace ReverseDebugger {
namespace Internal {

ReverseDebuggerMgr::ReverseDebuggerMgr(QObject *parent)
    : QObject(parent), runCtrl(new MinidumpRunControl(this))
{
}

void ReverseDebuggerMgr::recored()
{
    recordMinidump();
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
        runCtrl->start(emdParams);
    }
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
