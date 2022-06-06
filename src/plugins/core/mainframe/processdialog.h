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
#ifndef PROCESSDIALOG_H
#define PROCESSDIALOG_H

#include <QDialog>

class ProcessDialogPrivate;
class ProcessDialog : public QDialog
{
    Q_OBJECT
    ProcessDialogPrivate *const d;
public:
    explicit ProcessDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~ProcessDialog();
    static ProcessDialog* globalInstance();
    void setRunning(bool runable);
    ProcessDialog& operator << (const QString &message);
    void setTitle(const QString &title);
};

#endif // PROCESSDIALOG_H
