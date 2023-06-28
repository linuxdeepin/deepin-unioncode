// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGEWIDGET_H
#define PAGEWIDGET_H

#include <QWidget>

class PageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PageWidget(QWidget *parent = nullptr);
    virtual ~PageWidget() = 0;
    virtual void saveConfig(){}
    virtual void readConfig(){}

    virtual void setUserConfig(const QMap<QString, QVariant> &map){}
    virtual void getUserConfig( QMap<QString, QVariant> &map){}
};

#endif // PAGEWIDGET_H
