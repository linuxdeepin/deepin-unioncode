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
#ifndef BACKENDCHECKER_H
#define BACKENDCHECKER_H

#include <QWidget>
#include <QUrl>
#include <QProcess>
#include <QProgressBar>
#include <QDir>
#include <QTextBrowser>
#include <QCryptographicHash>

class RequestInfo
{
    QUrl packageUrl;
    QString packageSaveName;
    QUrl checkFileUrl;
    QString checkFileSaveName;
    QString checkNumProgram;
    QString checkNumMode;
public:
    RequestInfo() = default;
    RequestInfo(const QUrl &packageUrl,
                const QString &packageSaveName,
                const QUrl &checkFileUrl,
                const QString &checkFileSaveName,
                const QString &checkNumProgram,
                const QString &checkNumMode)
        : packageUrl(packageUrl),
          packageSaveName(packageSaveName),
          checkFileUrl(checkFileUrl),
          checkFileSaveName(checkFileSaveName),
          checkNumProgram(checkNumProgram),
          checkNumMode(checkNumMode){}

    QUrl getPackageUrl() const;
    QString getPackageSaveName() const;
    QUrl getCheckFileUrl() const;
    QString getCheckFileSaveName() const;
    QString getCheckNumProgram() const;
    QString getCheckNumMode() const;

    void setPackageUrl(const QUrl &value);
    void setPackageSaveName(const QString &value);
    void setCheckFileUrl(const QUrl &value);
    void setCheckFileSaveName(const QString &value);
    void setCheckNumProgram(const QString &value);
    void setCheckNumMode(const QString &value);
};
Q_DECLARE_METATYPE(RequestInfo)

struct Pip3GitInstall
{
    QString packageName;
    QString programMain;
};
Q_DECLARE_METATYPE(Pip3GitInstall)

class BackendChecker : public QWidget
{
    Q_OBJECT
public:
    explicit BackendChecker(QWidget *parent = nullptr);
    static BackendChecker &instance();
    void checkLanguageBackend(const QString &languageID);
    bool existRunMain(const QString &languageID);
    bool existShellRemain(const QString &languageID);
    bool createShellRemain(const QString &languageID);
    bool checkCachePackage(const QString &languageID);
private:
    QString adapterPath;
    QHash<QString, QVariant> requestInfos;
};

#endif // BACKENDCHECKER_H
