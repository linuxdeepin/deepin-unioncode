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
#include "cmakeopenhandler.h"
#include "services/window/windowservice.h"
#include <QFileDialog>

namespace {
CMakeOpenHandler *ins{ nullptr };
}

CMakeOpenHandler::CMakeOpenHandler(QObject *parent) : QObject(parent)
{

}

CMakeOpenHandler *CMakeOpenHandler::instance()
{
    if (!ins)
        ins = new CMakeOpenHandler;
    return ins;
}

QAction *CMakeOpenHandler::openAction()
{
    auto result = new QAction("cmake");
    QObject::connect(result, &QAction::triggered, [=](){
        QFileDialog fileDialog(nullptr, "Open CMake Project", QDir::homePath(), "CMakeLists.txt");
        if(fileDialog.exec()) {
            CMakeOpenHandler::projectOpened(result->text(), dpfservice::MWMFA_CXX, fileDialog.selectedFiles()[0]);
        }
    });
    return result;
}
