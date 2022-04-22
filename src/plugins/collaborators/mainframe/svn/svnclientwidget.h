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
#ifndef SVNCLIENTWIDGET_H
#define SVNCLIENTWIDGET_H

#include <QMainWindow>
#include <QToolBar>
#include <QSet>

class ReposWidget;
class QPinnableTabWidget;
class SvnClientWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit SvnClientWidget(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

signals:

public slots:
    void addRepoTab(const QString &repoPath);
    void addNewRepoTab(const QString &repoPathArg, bool pinned);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QSet<QString> mCurrentRepos;
    QPinnableTabWidget *mRepos{nullptr};
};

#endif // SVNCLIENTWIDGET_H
