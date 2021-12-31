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
#include "contextdialog.h"

#include <QAbstractButton>
#include <QMessageBox>
#include <QFile>

void ContextDialog::okCancel(QString text, QString title,
                             QMessageBox::Icon icon,
                             std::function<void (bool)> okCallBack,
                             std::function<void (bool)> cancelCallBack)
{
    QMessageBox messageBox;
    messageBox.setWindowTitle(title);
    messageBox.setText(text);
    messageBox.setIcon(icon);
    messageBox.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
    if (okCallBack)
        QObject::connect(messageBox.button(QMessageBox::Ok),
                         &QAbstractButton::clicked, okCallBack);
    if (cancelCallBack)
        QObject::connect(messageBox.button(QMessageBox::Cancel),
                         &QAbstractButton::clicked, cancelCallBack);
    messageBox.exec();
}

void ContextDialog::ok(QString text, QString title,
                       QMessageBox::Icon icon,
                       std::function<void (bool)> okCallBack)
{
    QMessageBox messageBox;
    messageBox.setWindowTitle(title);
    messageBox.setText(text);
    messageBox.setIcon(icon);
    messageBox.setStandardButtons(QMessageBox::Ok);
    if (okCallBack)
        QObject::connect(messageBox.button(QMessageBox::Ok),
                         &QAbstractButton::clicked, okCallBack);
    messageBox.exec();
}
