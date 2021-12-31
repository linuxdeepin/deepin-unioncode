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
#ifndef DETAILSBUTTON_H
#define DETAILSBUTTON_H

#include <QAbstractButton>

class DetailsButtonPrivate;
class DetailsButton : public QAbstractButton
{
    Q_OBJECT
    class DetailsButtonPrivate *const d;
public:
    explicit DetailsButton(QWidget *parent = nullptr);
    virtual ~DetailsButton() override;

protected:
    void paintEvent(QPaintEvent *e) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

#endif // DETAILSBUTTON_H
