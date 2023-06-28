// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowtheme.h"

#include <QApplication>
#include <QFile>
#include <QDebug>

void WindowTheme::setTheme(const QString &file)
{
    QFile qssFile(file);
    if (!qssFile.open(QFile::ReadOnly)) {
        qCritical()<< "Failed can't open qss file" << file;
        return;
    }
    qApp->setStyleSheet(qssFile.readAll());
}
