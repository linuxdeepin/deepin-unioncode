// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKENDCHECKER_H
#define BACKENDCHECKER_H

#include "framework/framework.h"

#include <QWidget>
#include <QUrl>
#include <QProcess>
#include <QProgressBar>
#include <QDir>
#include <QTextBrowser>
#include <QCryptographicHash>

class RemoteChecker
{
public:
    explicit RemoteChecker();
    static RemoteChecker &instance();
    void checkLanguageBackend(const QString &language);
    bool checkShasum(const QString &filePath, const QString &src_code, const QString &mode);
    QString getRemoteFile(const QUrl &url);
    bool saveRemoteFile(const QUrl &url, const QString &saveFilePath);
    void checkJSServer(const QString &checkPath);

private:
    void doCheckClangd(const QString &language);
    void doCheckJdtls(const QString &language);
    void doCheckPyls(const QString &language);

    bool checkClangdFlag = false;
    bool checkJdtlsFlag = false;
    bool checkPylsFlag = false;
    bool checkJSServerFlag = false;
};

#endif // BACKENDCHECKER_H
