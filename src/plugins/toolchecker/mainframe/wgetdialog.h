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
#ifndef WGETDIALOG_H
#define WGETDIALOG_H

#include "processdialog.h"

class WGetDialog : public ProcessDialog
{
    Q_OBJECT
public:
    WGetDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    void setWorkDirectory(const QString &workDir);
    QString workDirectory() const;
    void setWgetArguments(const QStringList &list);
    QStringList wgetArguments() const;

protected:
    virtual void doShowStdErr(const QByteArray &array) override;
    virtual void doShowStdOut(const QByteArray &array) override;
    virtual void doFinished(int exitCode, QProcess::ExitStatus status) override;
    virtual void showEvent(QShowEvent *event) override;
};

#endif // WGETDIALOG_H
