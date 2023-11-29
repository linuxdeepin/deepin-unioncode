// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jsontabwidget.h"
#include "jsondisplayview.h"

#include <QStandardItemModel>

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
