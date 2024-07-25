// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOWNLOADUTIL_H
#define DOWNLOADUTIL_H

#include <QObject>
#include <memory>
#include <QFile>

class DownloadUtilPrivate;
class DownloadUtil : public QObject
{
    Q_OBJECT
public:
    explicit DownloadUtil(const QString &srcUrl,
                          const QString &dstPath,
                          const QString &fileName,
                          QObject *parent = nullptr);
    virtual ~DownloadUtil();

    bool start();
    void cancel();

signals:
    void sigProgress(qint64 bytesRead, qint64 totalBytes);
    void sigFinished();
    void sigFailed();

private:
    void startRequest(const QUrl &url);
    std::unique_ptr<QFile> openFileForWrite(const QString& fileName);

private:
    DownloadUtilPrivate *const d;


};

#endif // DOWNLOADUTIL_H

