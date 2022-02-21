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
#ifndef TEXTEDITTITLEBAR_H
#define TEXTEDITTITLEBAR_H

#include <QStatusBar>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

#include <iostream>
#include <functional>

class TextEditTitleBarPrivate;
class TextEditTitleBar : public QStatusBar
{
    Q_OBJECT
    TextEditTitleBarPrivate * const d;

public:

    enum StandardButton
    {
        Reload,
        Cancel
    };

    explicit TextEditTitleBar(QWidget *parent = nullptr);
    virtual ~TextEditTitleBar();
    QPushButton * button(StandardButton button);
    static TextEditTitleBar* changedReload(const QString &filePath);
};

#endif // TEXTEDITTITLEBAR_H
