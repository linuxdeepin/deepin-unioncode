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
#ifndef PIP3DIALOG_H
#define PIP3DIALOG_H

#include "processdialog.h"

class Pip3Dialog : public ProcessDialog
{
    Q_OBJECT
public:
    Pip3Dialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    void install(const QString &packageName);
    void uninstall(const QString &packageName);
protected:
    virtual void doShowStdErr(const QByteArray &array);
};

#endif // PIP3DIALOG_H
