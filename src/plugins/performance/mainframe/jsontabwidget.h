// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JSONTABWIDGET_H
#define JSONTABWIDGET_H

#include <json/json.h>

#include <QTabWidget>

class JsonTabWidgetPrivate;
class JsonTabWidget : public QTabWidget
{
    Q_OBJECT
    JsonTabWidgetPrivate *const d;
public:
    explicit JsonTabWidget(QWidget *parent = nullptr);
    virtual ~JsonTabWidget();

public slots:
    void parseJson(const Json::Value &data);
};

#endif // JSONTABWIDGET_H
