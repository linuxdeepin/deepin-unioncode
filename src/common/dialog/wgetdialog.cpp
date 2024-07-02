// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wgetdialog.h"

#include <QRegularExpression>
#include <QDebug>

WGetDialog::WGetDialog(QWidget *parent)
    : ProcessDialog (parent)
{
    setWindowTitle("wget request download");
    process.setProgram("wget");
}

void WGetDialog::setWorkingDirectory(const QString &workDir)
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
    return DAbstractDialog::showEvent(event);
}
