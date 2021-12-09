/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#include "backtrace.h"

#include <qlogging.h>
#include <QCoreApplication>

#include <csignal>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <sstream>

#define TOSTRING(s) #s

DPF_BEGIN_NAMESPACE
namespace backtrace {

/*!
 * \brief demangle ABI-mandated entry point in the
 * C++ runtime library for demangling
 * \param value backtrace string
 * \return demangled value
 */
std::string demangle(void *value)
{
    if (!value)
        return "";

    std::ostringstream ostream;
    ostream.imbue(std::locale::classic());
    ostream << value << " : ";
    Dl_info info = {nullptr, nullptr, nullptr, nullptr};
    if (dladdr(value, &info) == 0) {
        ostream << "???";
    } else {
        if (info.dli_sname) {
            int status = 0;
            char *demangledName = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
            if (demangledName) {
                ostream << demangledName;
                free(demangledName);
            } else {
                ostream << info.dli_sname;
            }
        } else {
            ostream << "???";
        }

        long offset = reinterpret_cast<char *>(value) - reinterpret_cast<char *>(info.dli_saddr);
        ostream << std::hex << " + 0x" << offset ;

        if (info.dli_fname)
            ostream << " @ " << info.dli_fname;
    }
    return ostream.str();
}

/*!
 * \brief logStackInfo
 * \param signal kernel expection signal
 */
[[noreturn]] void logStackInfo(int signal)
{
    const int bufSize = 100;
    void *buffer[bufSize] = {nullptr};
    int numLine = ::backtrace(buffer, bufSize);

    std::string strSig;
    switch (signal) {
    case SIGINT: /* Interactive attention signal.  */
        strSig = TOSTRING(SIGINT);
        break;
    case SIGILL: /* Illegal instruction.  */
        strSig = TOSTRING(SIGILL);
        break;
    case SIGABRT:  /* Abnormal termination.  */
        strSig = TOSTRING(SIGABRT);
        break;
    case SIGFPE: /* Erroneous arithmetic operation.  */
        strSig = TOSTRING(SIGFPE);
        break;
    case SIGSEGV: /* Invalid access to storage.  */
        strSig = TOSTRING(SIGSEGV);
        break;
    case SIGTERM: /* Termination request.  */
        strSig = TOSTRING(SIGTERM);
        break;
    default:
        char szTmpBuf[bufSize] = {0};
        sprintf(szTmpBuf, "No register signal: %d", signal);
        strSig = szTmpBuf;
        break;
    };
    QString head,end;
    head = QString("****************** %0 crashed backtrace ******************")
            .arg(qApp->applicationName());
    qCritical("%s", head.toStdString().data());
    qCritical("* signal:%s numLine:%d", strSig.data(), numLine);
    for (int i = 1; i < numLine; ++i) {
        std::string stackInfo = demangle(buffer[i]);
        qCritical("* %d>  %s", i, stackInfo.data());
    }
    for(int index = head.size(); index > 0; index --) {
        end += "*";
    }
    qCritical("%s", end.toStdString().data());
    exit(EXIT_FAILURE);
}

/*!
 * \brief regSignal
 * register sig signal handler.
 * \param sig
 */
void regSignal(int sig)
{
    signal(sig, logStackInfo);
}

/*!
 * \brief initbacktrace
 * Register signal handler.
 */
void initbacktrace()
{
    regSignal(SIGINT); /* Interactive attention signal.  */
    regSignal(SIGILL); /* Illegal instruction.  */
    regSignal(SIGABRT); /* Abnormal termination.  */
    regSignal(SIGFPE); /* Erroneous arithmetic operation.  */
    regSignal(SIGSEGV); /* Invalid access to storage.  */
    regSignal(SIGTERM); /* Termination request.  */
}
}
DPF_END_NAMESPACE
