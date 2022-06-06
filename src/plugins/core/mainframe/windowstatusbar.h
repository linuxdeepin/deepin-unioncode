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
#ifndef WINDOWSTATUSBAR_H
#define WINDOWSTATUSBAR_H

#include <QStatusBar>

class WindowStatusBarPrivate;
class WindowStatusBar : public QStatusBar
{
    Q_OBJECT
    WindowStatusBarPrivate *const d;
public:
    explicit WindowStatusBar(QWidget *parent = nullptr);
    virtual ~WindowStatusBar();
    void setPercentage(int percentage);
    void setMessage(const QString &message);
    bool progressIsHidden();
    void hideProgress();
    void showProgress();
};

#endif // WINDOWSTATUSBAR_H
