/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#include "abstractnav.h"
#include "abstractmenubar.h"
#include "abstractmainwindow.h"

#include <QDebug>
#include <QObject>
#include <QMainWindow>
#include <QToolBar>
#include <QMenuBar>

class AbstractMainWindowPrivate
{
    friend class AbstractMainWindow;
    QMainWindow* mainwindow = nullptr;
};

AbstractMainWindow::~AbstractMainWindow()
{
    if (d) {
        delete d;
    }
}

void *AbstractMainWindow::qMainWindow()
{
    return (void*)(d->mainwindow);
}

AbstractMainWindow::AbstractMainWindow(void *qMainWindow)
    : d(new AbstractMainWindowPrivate())
{
    if (!qMainWindow) {
        qCritical() << "Failed, use QMainWindow(0x0) to AbstractMainWindow";
        abort();
    }

    d->mainwindow = (QMainWindow*)qMainWindow;
    QMainWindow::connect(d->mainwindow, &QMainWindow::destroyed,
                         d->mainwindow ,[this]() {
        delete this;
    }, Qt::UniqueConnection);
}
