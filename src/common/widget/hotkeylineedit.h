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

#ifndef HOTKEYLINEEDIT_H
#define HOTKEYLINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QFocusEvent>

class HotkeyLineEditPrivate;
class HotkeyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit HotkeyLineEdit(QWidget *parent = nullptr);
    virtual ~HotkeyLineEdit() override;
    void setKey(int key);
    int getKey();
    void setText(QString text);
    void setHotkeyMode(bool hotkeyMode);
    bool isHotkeyMode();

signals:
    void sigKeyChanged(int key);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    HotkeyLineEditPrivate *const d;
};

#endif // HOTKEYLINEEDIT_H
