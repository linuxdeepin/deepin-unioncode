// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>
#include <QFileInfo>

namespace Utils {
class FileName : private QString
{
public:
    FileName();

    explicit FileName(const QFileInfo &info);

    static FileName fromUserInput(const QString &filename);

    const QString &toString() const;

    bool exists() const;

private:
    explicit FileName(const QString &string);
};
}

#endif // FILEUTILS_H
