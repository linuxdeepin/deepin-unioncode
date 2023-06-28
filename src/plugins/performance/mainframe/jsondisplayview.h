// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JSONDISPLAYVIEW_H
#define JSONDISPLAYVIEW_H

#include <json/json.h>

#include <QTableView>

class JsonDisplayViewPrivate;
class JsonDisplayView : public QTableView
{
    Q_OBJECT
    JsonDisplayViewPrivate *const d;
public:
    explicit JsonDisplayView(QWidget *parent = nullptr);
    virtual ~JsonDisplayView();
public slots:
    void parseJson(const Json::Value &jsonObj);
};

#endif // JSONDISPLAYVIEW_H
