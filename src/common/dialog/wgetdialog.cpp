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
#include "wgetdialog.h"

#include <QRegularExpression>
#include <QDebug>

WGetDialog::WGetDialog(QWidget *parent, Qt::WindowFlags f)
    : ProcessDialog (parent, f)
{
    setWindowTitle("wget request download");
    process.setProgram("wget");
}

void WGetDialog::setWorkDirectory(const QString &workDir)
{
    process.setWorkingDirectory(workDir);
}

QString WGetDialog::workDirectory() const
{
    return process.workingDirectory();
}

void WGetDialog::setWgetArguments(const QStringList &list)
{
    setWindowTitle(windowTitle() + " " + list.join(" "));
    process.setArguments(list);
}

QStringList WGetDialog::wgetArguments() const
{
    return process.arguments();
}

void WGetDialog::doShowStdErr(const QByteArray &array)
{
    static QString cacheData;
    static QString headTitle;
    static QString downloadLine;
    cacheData += array;
    auto datas = cacheData.split("\n\n");
    if (datas.size() >= 2) {
        headTitle = datas.takeAt(0);
        textBrowser->setText(headTitle + "\n" + downloadLine);
        cacheData = datas.join("");
    }

    if (!headTitle.isEmpty()) {
        auto lines = cacheData.split("\n");
        if (lines.size() >= 2) {
            downloadLine = lines.takeAt(0);
            textBrowser->setText(headTitle + "\n" + downloadLine);
            QRegularExpression regExp("\\d+\\%");
            auto matchRes = regExp.match(downloadLine);
            if (matchRes.hasMatch()) {
                QString matched = matchRes.captured();
                matched = matched.remove(matched.size() - 1, 1);
                doShowProgress(matched.toInt(), 100);
            }
            cacheData = lines.join("");
        }
    }
}

void WGetDialog::doShowStdOut(const QByteArray &array)
{
    textBrowser->append(array);
}

void WGetDialog::doFinished(int exitCode, QProcess::ExitStatus status)
{
    this->close();
    qInfo() << exitCode << status;
}

void WGetDialog::showEvent(QShowEvent *event)
{
    process.start();
    return QDialog::showEvent(event);
}
