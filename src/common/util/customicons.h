// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMICON_H
#define CUSTOMICON_H

#include <QFileIconProvider>

class CustomIcons final
{
    CustomIcons() = delete;
    Q_DISABLE_COPY(CustomIcons)
public:
    enum CustomIconType { Exe, Lib };
    static QIcon icon(QFileIconProvider::IconType type);
    static QIcon icon(CustomIconType type);
    static QIcon icon(const QFileInfo &info);
    static QString type(const QFileInfo &info);
    static void setOptions(QFileIconProvider::Options options);
    static QFileIconProvider::Options options();
};

#endif // CUSTOMICON_H
