// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JAVAUTIL_H
#define JAVAUTIL_H

#include <QObject>
#include <QDir>

class JavaUtil : public QObject
{
    Q_OBJECT
public:
    static QString getMainClassPath(const QDir &dir);
    static QString getMainClass(const QString &mainClassPath, const QString &packageDirName);
    static QString getPackageDir(const QString &mainClassPath, const QString &packageDirName);

signals:

public slots:

private:
    explicit JavaUtil(QObject *parent = nullptr) = delete;
};

#endif // JAVAUTIL_H
