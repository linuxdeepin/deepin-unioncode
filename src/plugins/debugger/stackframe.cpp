// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stackframe.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

////////////////////////////////////////////////////////////////////////
//
// StackFrame
//
////////////////////////////////////////////////////////////////////////

StackFrameData::StackFrameData() = default;

void StackFrameData::clear()
{
    line = -1;
    function.clear();
    file.clear();
    module.clear();
    receiver.clear();
    address = "";
}

bool StackFrameData::isUsable() const
{
    return usable;
}

QString StackFrameData::toString() const
{
    QString res;
    QTextStream str(&res);
    str << tr("Address:") << ' ';
    str.setIntegerBase(16);
    str << address;
    str.setIntegerBase(10);
    str << ' '
        << tr("Function:") << ' ' << function << ' '
        << tr("File:") << ' ' << file << ' '
        << tr("Line:") << ' ' << line << ' '
        << tr("From:") << ' ' << module << ' '
        << tr("To:") << ' ' << receiver;
    return res;
}

QString StackFrameData::toToolTip() const
{
    const QString filePath = QDir::toNativeSeparators(file);
    QString res;
    QTextStream str(&res);
    str << "<html><body><table>";
    if (!address.isEmpty())
        str << "<tr><td>" << tr("Address:") << "</td><td>"
            << /*formatToolTipAddress(address)*/address << "</td></tr>";
    if (!function.isEmpty())
        str << "<tr><td>"
            << tr("Function:")
            << "</td><td>" << function << "</td></tr>";
    if (!file.isEmpty())
        str << "<tr><td>" << tr("File:") << "</td><td>" << filePath << "</td></tr>";
    if (line != -1)
        str << "<tr><td>" << tr("Line:") << "</td><td>" << line << "</td></tr>";
    if (!module.isEmpty())
        str << "<tr><td>" << tr("Module:") << "</td><td>" << module << "</td></tr>";
    if (!receiver.isEmpty())
        str << "<tr><td>" << tr("Receiver:") << "</td><td>" << receiver << "</td></tr>";
    str << "</table>";

    str <<"<br> <br><i>" << tr("Note:") << " </i> ";
    if (isUsable()) {
        str << tr("You can double click it to reatch source.");
    } else if (line <= 0) {
        str << tr("Binary debug information is not accessible for this "
            "frame. This either means the core was not compiled "
            "with debug information, or the debug information is not "
            "accessible.");
    } else {
        str << tr("Binary debug information is accessible for this "
            "frame. However, matching sources have not been found.");
    }

    str << "</body></html>";
    return res;
}

QDebug operator<<(QDebug d, const  StackFrameData &f)
{
    QString res;
    QTextStream str(&res);
    str << "level=" << f.level << " address=" << f.address;
    if (!f.function.isEmpty())
        str << ' ' << f.function;
    if (!f.file.isEmpty())
        str << ' ' << f.file << ':' << f.line;
    if (!f.module.isEmpty())
        str << " from=" << f.module;
    if (!f.receiver.isEmpty())
        str << " to=" << f.receiver;
    d.nospace() << res;
    return d;
}
