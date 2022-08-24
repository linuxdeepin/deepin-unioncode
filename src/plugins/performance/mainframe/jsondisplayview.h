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
#ifndef JSONDISPLAYVIEW_H
#define JSONDISPLAYVIEW_H

#include <jsoncpp/json/json.h>

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
