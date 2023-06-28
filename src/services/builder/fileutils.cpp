// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileutils.h"

#include <QDir>

namespace Utils {
FileName::FileName()
{

}

FileName::FileName(const QFileInfo &info)
    : QString(info.absoluteFilePath())
{
}

FileName FileName::fromUserInput(const QString &filename)
{
    QString clean = QDir::cleanPath(filename);
    if (clean.startsWith(QLatin1String("~/")))
        clean = QDir::homePath() + clean.mid(1);
    return FileName(clean);
}

const QString &FileName::toString() const
{
    return *this;
}

bool FileName::exists() const
{
    return !isEmpty() && QFileInfo::exists(*this);
}

FileName::FileName(const QString &string)
    : QString(string)
{

}

}
