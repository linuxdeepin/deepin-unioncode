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

class TextEditTabBarPrivate;
class TextEditTabBar : public QWidget
{
    Q_OBJECT
    friend class TextEditTabBarPrivate;
    TextEditTabBarPrivate *const d;
public:
    explicit TextEditTabBar(QWidget *parent = nullptr);
    virtual ~TextEditTabBar();
    int fileIndex(const QString &file) const;
    QString indexFile(int index) const;
    void setFile(const QString &file);
    void switchFile(const QString &file);
    void removeTab(const QString &file);
    int count() const;
    int currentIndex() const;
    QString currentFile() const;
    void setCurrentIndex(int idx);
    void setCloseButtonVisible(bool flag);
    void setSplitButtonVisible(bool flag);

signals:
    void fileClosed(const QString &file);
    void fileSwitched(const QString &file);
    void saveFile(const QString &file);
    void splitClicked(Qt::Orientation ori);
    void closeClicked();

public slots:
    void tabCloseRequested(int idx);
    void doFileChanged(const QString &file);
    void doFileSaved(const QString &file);
};

#endif // TEXTEDITTABBAR_H
