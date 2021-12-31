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
#ifndef EDITFILESTATUSBAR_H
#define EDITFILESTATUSBAR_H

#include <QStatusBar>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

#include <iostream>
#include <functional>

class EditFileStatusBarPrivate;
class EditFileStatusBar : public QStatusBar
{
    Q_OBJECT
    EditFileStatusBarPrivate * const d;

public:

    enum StandardButton
    {
        Reload,
        Cancel
    };

    explicit EditFileStatusBar(QWidget *parent = nullptr);
    virtual ~EditFileStatusBar();
    QPushButton * button(StandardButton button);
    static EditFileStatusBar* changedReload(const QString &filePath);
};

#endif // EDITFILESTATUSBAR_H
