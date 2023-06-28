// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <QString>
#include <QJsonDocument>

class Configure
{
    Configure() = delete;
    Configure(const Configure &other) = delete;
public:
    static bool enabled();
    static void setEnabled(bool enabled);
private:
    static QString filePath();
    static QJsonDocument doc();
};

#endif // CONFIGURE_H
