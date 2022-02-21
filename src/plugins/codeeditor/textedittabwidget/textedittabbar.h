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
#ifndef TEXTEDITTABBAR_H
#define TEXTEDITTABBAR_H

#include <QTabBar>

class TextEditTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit TextEditTabBar(QWidget *parent = nullptr);
    int fileIndex(const QString &file);
    QString indexFile(int index);
    void setFile(const QString &file);
    void switchFile(const QString &file);
    void removeTab(const QString &file);

signals:
    void fileClosed(const QString &file);
    void fileSwitched(const QString &file);

public slots:
    void doFileChanged(const QString &file);
    void doFileSaved(const QString &file);
};

#endif // TEXTEDITTABBAR_H
