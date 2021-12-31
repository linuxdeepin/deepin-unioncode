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
#ifndef EDITWIDGET_H
#define EDITWIDGET_H

#include <QWidget>

class EditWidgetPrivate;
class EditWidget : public QWidget
{
    Q_OBJECT
    EditWidgetPrivate *const d;
public:
    explicit EditWidget(QWidget *parent = nullptr);
    virtual ~EditWidget();

public slots:
    void openFile(const QString &filePath, const QString &workspaceFolder);
    void closeFile(const QString &filePath);

private:
    int tabIndex(const QString &tabTooltip);

private slots:
    void setDefaultFileEdit();
    void hideFileEdit(int tabIndex);
    void showFileEdit(int tabIndex);
    void hideFileStatusBar(int tabIndex);
    void showFileStatusBar(int tabIndex);
    void removeFileStatusBar(int tabIndex);
    void removeFileEdit(int tabIndex);
    void removeFileTab(int tabIndex);
    void fileModifyed(const QString &file);
    void fileDeleted(const QString &file);
    void fileMoved(const QString &file);
};

#endif // EDITWIDGET_H
