/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#ifndef SETTINGINFO_H
#define SETTINGINFO_H

#include <QStringList>
#include <QDebug>

struct SettingInfo
{
    QString language;
    QString mode;
    QString program;
    QStringList arguments;
    QString workDir;
};

inline QDebug operator << (QDebug debug, const SettingInfo &info) {
    debug << "SettingInfo("
          << info.language
          << info.mode
          << info.program
          << info.arguments
          << info.workDir
          << ")";
    return debug;
}

inline std::ostream& operator <<(std::ostream &os, const SettingInfo &info) {
    os << "SettingInfo("
       << info.language.toStdString() << ","
       << info.mode.toStdString() << ","
       << info.program.toStdString() << ","
       << "(" << info.arguments.join(",").toStdString() << ")" << ","
       << info.workDir.toStdString()
       << ")";
    return os;
}

#endif // SETTINGINFO_H
