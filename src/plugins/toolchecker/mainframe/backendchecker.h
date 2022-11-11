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

class BackendChecker : public QWidget
{
    Q_OBJECT
public:
    explicit BackendChecker(QWidget *parent = nullptr);
    static BackendChecker &instance();
    void checkLanguageBackend(const QString &language);
    bool checkShasum(const QString &filePath, const QString &src_code, const QString &mode);
    QString getRemoteFile(const QUrl &url);
    bool saveRemoteFile(const QUrl &url, const QString &saveFilePath);
    QString localPlatform();
private:
    void doCheckClangd(const QString &language);
    void doCheckJdtls(const QString &language);
    void doCheckPyls(const QString &language);
};

#endif // BACKENDCHECKER_H
