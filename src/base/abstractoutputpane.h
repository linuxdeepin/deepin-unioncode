/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#ifndef ABSTRUCTOUTPUTPANE_H
#define ABSTRUCTOUTPUTPANE_H

#include <QObject>

class AbstractOutputPane : public QObject
{
    Q_OBJECT
public:
    explicit AbstractOutputPane(QObject *parent = nullptr);
    virtual ~AbstractOutputPane() override;

    virtual QWidget *outputWidget(QWidget *parent) = 0;
    virtual void clearContents() = 0;

    virtual void setFocus() = 0;
    virtual bool hasFocus() const = 0;
    virtual bool canFocus() const = 0;

    virtual bool canNext() const = 0;
    virtual bool canPrevious() const = 0;
    virtual void goToNext() = 0;
    virtual void goToPrev() = 0;
};

#endif // ABSTRUCTOUTPUTPANE_H
