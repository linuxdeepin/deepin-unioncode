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
#ifndef NAVEDITWIDGET_H
#define NAVEDITWIDGET_H

#include <QSplitter>

class QGridLayout;
class QTabWidget;
class AbstractWidget;
class AbstractConsole;
class NavEditFrame final : public QSplitter
{
    Q_OBJECT
public:
    explicit NavEditFrame(QWidget *parent = nullptr);

public slots:
    void setTreeWidget(AbstractWidget *treeWidget);
    void setConsole(AbstractConsole *console);
    void setWatchWidget(AbstractWidget *watchWidget);
    void addContextWidget(const QString &title, AbstractWidget *contextWidget);

private:
    void createEditorWidget(QSplitter *splitter);
    void createContextWidget(QSplitter *splitter);
    QGridLayout *editorLayout = nullptr;
    QTabWidget *tabWidget = nullptr;
    QSplitter *verSplitter = nullptr;
    QSplitter *horSplitter = nullptr;
    QWidget *watchWidget = nullptr;
    QWidget *treeWidget = nullptr;
protected:
    bool eventFilter(QObject *obj, QEvent *e);
};

#endif // NAVEDITWIDGET_H
