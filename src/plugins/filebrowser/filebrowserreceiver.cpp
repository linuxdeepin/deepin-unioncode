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
#include "filebrowserreceiver.h"
#include "treeproxy.h"

static QStringList subTopics{"Menu"};

FileBrowserReceiver::FileBrowserReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<FileBrowserReceiver> ()
{

}

QStringList FileBrowserReceiver::topics()
{
    return subTopics; //绑定menu 事件
}

void FileBrowserReceiver::eventProcess(const dpf::Event &event)
{
    if (!subTopics.contains(event.topic()))
        abort();
    qInfo() << event;
    if (event.topic() == "Menu") {
        if (event.data() == "File.OpenDocument")
            return TreeProxy::instance().appendFile
                    (event.property("FilePath").toString());
        if (event.data() == "File.OpenFolder")
            return TreeProxy::instance().appendFolder
                    (event.property("FilePath").toString());
    }

}
