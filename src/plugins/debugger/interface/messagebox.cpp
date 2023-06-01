/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "messagebox.h"

#include <QMessageBox>
#include <QApplication>
#include <QWidget>

namespace Internal {

namespace {

QWidget *message(QMessageBox::Icon icon, const QString &title, const QString &desciption)
{
    QMessageBox *messageBox = new QMessageBox(icon,
                                              title,
                                              desciption,
                                              QMessageBox::Ok/*,
                                              dialogParent()*/);

    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    messageBox->setModal(true);
    messageBox->show();
    return messageBox;
}
}

QWidget *warning(const QString &title, const QString &desciption)
{
    return message(QMessageBox::Warning, title, desciption);
}

QWidget *information(const QString &title, const QString &desciption)
{
    return message(QMessageBox::Information, title, desciption);
}

QWidget *critical(const QString &title, const QString &desciption)
{
    return message(QMessageBox::Critical, title, desciption);
}
}
