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
#ifndef FILEBROWSERRECEIVER_H
#define FILEBROWSERRECEIVER_H

#include <framework/framework.h>

class FileBrowserReceiver: public dpf::EventHandler, dpf::AutoEventHandlerRegister<FileBrowserReceiver>
{
    friend class dpf::AutoEventHandlerRegister<FileBrowserReceiver>;
public:
    explicit FileBrowserReceiver(QObject * parent = nullptr);

    static Type type()
    {
        return dpf::EventHandler::Type::Sync;
    }

    static QStringList topics();

    void eventProcess(const dpf::Event &event) override;

private:
    void projectEvent(const dpf::Event &event);
};

#endif // FILEBROWSERRECEIVER_H
