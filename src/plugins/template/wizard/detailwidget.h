/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#ifndef DETAILWIDGET_H
#define DETAILWIDGET_H

#include "projectgenerate.h"

#include <QScrollArea>

using namespace templateMgr;

class DetailWidgetPrivate;
class DetailWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit DetailWidget(QWidget *parent = nullptr);
    DetailWidget(const QString &templatePath, QWidget *parent = nullptr);
    ~DetailWidget() override;

    bool getGenParams(PojectGenParam &param);

signals:

private slots:

private:
    DetailWidgetPrivate *const d;
};

#endif // DETAILWIDGET_H
