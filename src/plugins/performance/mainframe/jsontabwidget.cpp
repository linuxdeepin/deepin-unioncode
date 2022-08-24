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
#include "jsontabwidget.h"
#include "jsondisplayview.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QStandardItemModel>
#include <QDebug>

class JsonTabWidgetPrivate
{
    friend class JsonTabWidget;
    int findTab(QTabWidget *const tabWidget, const std::string tabText) const
    {
        for(int index = 0; index < tabWidget->count(); index ++){
            if (tabWidget->tabText(index).toStdString() == tabText)
                return index;
        }
        return -1;
    }
};

JsonTabWidget::JsonTabWidget(QWidget *parent)
    : QTabWidget (parent)
    , d (new JsonTabWidgetPrivate)
{
    setTabPosition(QTabWidget::TabPosition::South);
    qRegisterMetaType<Json::Value>("Json::Value");
    qRegisterMetaType<Json::Value>("Json::Value&");
}

JsonTabWidget::~JsonTabWidget()
{
    if (d)
        delete d;
}

void JsonTabWidget::parseJson(const Json::Value &data)
{
    if (!data.empty()) {
        Json::Value result = data["result"];
        if (!result.empty()) {
            for (auto name : result.getMemberNames()) {
                if (name.empty())
                    continue;
                int tabIdx = d->findTab(this, name);
                if (0 > tabIdx) {
                    tabIdx = addTab(new JsonDisplayView(this), QString::fromStdString(name));
                }
                JsonDisplayView *curr = qobject_cast<JsonDisplayView*>(widget(tabIdx));
                if (curr) {
                    curr->parseJson(result[name]);
                }
            }
        }
    }
}
