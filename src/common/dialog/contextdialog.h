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
#ifndef CONTEXTDIALOG_H
#define CONTEXTDIALOG_H

#include <QIcon>
#include <QString>
#include <QMessageBox>
#include <functional>

class ContextDialog final
{
    Q_DISABLE_COPY(ContextDialog)
    ContextDialog() = delete;
public:
    static void okCancel(QString text,
                         QString title = "Warning",
                         QMessageBox::Icon icon = QMessageBox::Warning,
                         std::function<void(bool)> okCallBack = nullptr,
                         std::function<void(bool)> cancelCallBack = nullptr);

    static void ok(QString text,
                   QString title = "Error",
                   QMessageBox::Icon icon = QMessageBox::Critical,
                   std::function<void(bool)> okCallBack = nullptr);
};

#endif
