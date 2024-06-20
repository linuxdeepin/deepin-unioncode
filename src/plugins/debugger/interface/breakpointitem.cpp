// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "breakpointitem.h"
#include "breakpoint.h"

#include <QDir>
#include <QTextStream>

using namespace Internal;

const QString empty(QLatin1Char('-'));

BreakpointItem::BreakpointItem(const Internal::Breakpoint &_bp)
    : bp(_bp)
{

}

BreakpointItem::BreakpointItem(const BreakpointItem &item)
    : bp(item.breakpoint())
{
}

BreakpointItem::BreakpointItem()
{
}

BreakpointItem::~BreakpointItem()
{

}

QVariant BreakpointItem::data(int row, int column, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (column) {
            case kIndexColumn:
                return QString::number(row + 1);
            case kFunctionNameColumn:
                return empty;
            case kFileNameColumn:
                return bp.fileName.isEmpty() ? empty : QDir::toNativeSeparators(bp.fileName);
            case kLineNumberColumn:
                return bp.lineNumber > 0 ? QString::number(bp.lineNumber) : empty;
            case kAddressColumn:
                return bp.address;
        }
    }

    if (role == Qt::DecorationRole && column == kIsEnabled) {
        return bp.enabled ? QIcon::fromTheme("breakpoint") : QIcon::fromTheme("disabled_breakpoint");
    }

    if (role == Qt::ToolTipRole)
        return toolTip();

    return QVariant();
}

QString BreakpointItem::markerFileName() const
{
    return bp.fileName;
}

int BreakpointItem::markerLineNumber() const
{
    return bp.lineNumber;
}

int BreakpointItem::modelId() const
{
    return 0;
}

static QString msgBreakpointAtSpecialFunc(const QString &func)
{
    return BreakpointItem::tr("Breakpoint at \"%1\"").arg(func);
}

static QString typeToString(BreakpointType type)
{
    switch (type) {
        case BreakpointByFileAndLine:
            return BreakpointItem::tr("Breakpoint by File and Line");
        case BreakpointByFunction:
            return BreakpointItem::tr("Breakpoint by Function");
        case BreakpointByAddress:
            return BreakpointItem::tr("Breakpoint by Address");
        case BreakpointAtThrow:
            return msgBreakpointAtSpecialFunc("throw");
        case BreakpointAtCatch:
            return msgBreakpointAtSpecialFunc("catch");
        case BreakpointAtExec:
            return msgBreakpointAtSpecialFunc("exec");
        //case BreakpointAtVFork:
        //    return msgBreakpointAtSpecialFunc("vfork");
        case UnknownBreakpointType:
        case LastBreakpointType:
            break;
    }
    return BreakpointItem::tr("Unknown Breakpoint Type");
}

QString BreakpointItem::toolTip() const
{
    QString rc;
    QTextStream str(&rc);
    str << "<html><body><b>" << BreakpointItem::tr("Unclaimed Breakpoint") << "</b>"
        << "<table>"
        //<< "<tr><td>" << tr("ID:") << "</td><td>" << m_id << "</td></tr>"
        << "<tr><td>" << BreakpointItem::tr("State:")
        << "</td><td>" << (bp.enabled ? BreakpointItem::tr("Enabled") : BreakpointItem::tr("Disabled"))
        << "<tr><td>" << BreakpointItem::tr("Breakpoint Type:")
        << "</td><td>" << typeToString(bp.type) << "</td></tr>";
    if (bp.type == BreakpointByFunction) {
        str << "<tr><td>" << BreakpointItem::tr("Function Name:")
        << "</td><td>" << bp.functionName
        << "</td></tr>";
    }
    if (bp.type == BreakpointByFileAndLine) {
        str << "<tr><td>" << BreakpointItem::tr("File Name:")
            << "</td><td>" << QDir::toNativeSeparators(bp.fileName)
            << "</td></tr>"
            << "<tr><td>" << BreakpointItem::tr("Line Number:")
            << "</td><td>" << bp.lineNumber;
    }
    if (bp.type == BreakpointByFunction || bp.type == BreakpointByFileAndLine) {
        str << "<tr><td>" << BreakpointItem::tr("Module:")
            << "</td><td>" << bp.module
            << "</td></tr>";
    }
    str << "<tr><td>" << BreakpointItem::tr("Breakpoint Address:") << "</td><td>";
    str << bp.address;
    str << "</td></tr>";

    str  << "</table></body></html><hr>";
    return rc;
}

int BreakpointItem::lineNumber() const
{
    return bp.lineNumber;
}

bool BreakpointItem::isEnabled() const
{
    return bp.enabled;
}

void BreakpointItem::setEnabled(bool on)
{
    bp.enabled = on;
}
