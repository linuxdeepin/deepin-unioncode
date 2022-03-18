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
#include "windowswitcher.h"
#include "windowkeeper.h"
#include "common/common.h"

#include <QGridLayout>
#include <QActionGroup>

void setNavActionChecked(const QString ActionName, bool checked)
{
    auto navActionGroup = WindowKeeper::instace()->navActionGroup();
    if (navActionGroup) {
        if (navActionGroup->actions().size() > 0) {
            for (auto action : navActionGroup->actions()) {
                if (action->text() == ActionName){
                    action->setChecked(checked);
                }
            }
        }
    }
}

WindowSwitcher::WindowSwitcher(QObject *parent)
    : dpf::EventHandler (parent)
{

}

dpf::EventHandler::Type WindowSwitcher::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList WindowSwitcher::topics()
{
    return QStringList() << "Nav";
}

void WindowSwitcher::eventProcess(const dpf::Event &event)
{
    if (!WindowSwitcher::topics().contains(event.topic())) {
        qCritical() << "Fatal error, unsubscribed message received";
        abort();
    }

    if (event.topic() == "Nav")
        navEvent(event);
}

void WindowSwitcher::navEvent(const dpf::Event &event)
{
    qInfo() << __FUNCTION__;
    auto centrals = WindowKeeper::instace()->centrals();
    auto window = WindowKeeper::instace()->mainWindow();
    for (const QString &navName : centrals.keys()) {
        QString data = event.data().toString();
        QWidget *widget = centrals.value(navName);
        if (!widget)
            continue;

        if (data.contains(navName) && data.contains("Show")) {
            if (window) {
                window->takeCentralWidget();
                window->setCentralWidget(widget);
            }
            setNavActionChecked(navName, true);
            widget->show();
        } else {
            centrals.value(navName)->hide();
        }
    }
    qInfo() << event;
}
