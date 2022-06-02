/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#ifndef FINDSENDER_H
#define FINDSENDER_H

#include <QString>
#include "services/project/projectservice.h"

class FindSender final : QObject
{
    Q_OBJECT
public:
    static void sendSearchCommand(const QString &searchText, int operateType);
    static void sendReplaceCommand(const QString &searchText, const QString &replaceText, int operateType);
    static void sendDoubleClickFileInfo(const QString &workspace, const QString &language, const QString &filePath, int line);

private:
    explicit FindSender(QObject *parent = nullptr);
    ~FindSender();
};

#endif // FINDSENDER_H
