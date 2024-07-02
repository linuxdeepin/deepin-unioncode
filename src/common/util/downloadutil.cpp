// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "downloadutil.h"

#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <memory>

class DownloadUtilPrivate {
    friend class DownloadUtil;
    QString srcUrl;
    QString dstPath;
    QString fileName;
    QString resultFileName;

    QNetworkAccessManager accessManager;
    QPointer<QNetworkReply> reply;

    std::unique_ptr<QFile> dstFile;
    bool requestAborted;
    QUrl requestUrl;
};

DownloadUtil::DownloadUtil(const QString &srcUrl,
                           const QString &dstPath,
                           const QString &fileName,
                           QObject *parent)
    : QObject(parent)
    , d(new DownloadUtilPrivate())
{
    d->srcUrl = srcUrl;
    d->dstPath = dstPath;
    d->fileName = fileName + ".downloading";
    d->resultFileName = fileName;

}

DownloadUtil::~DownloadUtil()
{

}

bool DownloadUtil::start()
{
    const QUrl srcUrl = QUrl::fromUserInput(d->srcUrl);

    if (!srcUrl.isValid() || d->dstPath.isEmpty() || d->fileName.isEmpty()) {
        return false;
    }

    if (!QFileInfo(d->dstPath).isDir()) {
        QDir dir;
        dir.mkpath(d->dstPath);
    }

    d->fileName.prepend(d->dstPath + '/');
    d->resultFileName.prepend(d->dstPath + '/');

    if (QFile::exists(d->fileName)) {
        QFile::remove(d->fileName);
    }

    d->dstFile = openFileForWrite(d->fileName);
    if (!d->dstFile)
        return false;

    startRequest(srcUrl);

    return true;
}


std::unique_ptr<QFile> DownloadUtil::openFileForWrite(const QString& fileName)
{
    std::unique_ptr<QFile> file(new QFile(fileName));
    if (!file->open(QIODevice::WriteOnly)) {

#ifdef DEBUG_INFO
        qInfo() << QString("Unable to save the file %1: %2.")
                    .arg(QDir::toNativeSeparators(fileName), file->errorString());
#endif

        return nullptr;
    }

    return file;
}


void DownloadUtil::startRequest(const QUrl& url)
{
    d->requestUrl = url;
    d->reply = d->accessManager.get(QNetworkRequest(url));

    connect(d->reply, &QNetworkReply::finished, [this](){
        QFileInfo file;
        if (d->dstFile) {
            file.setFile(d->dstFile->fileName());
            d->dstFile->close();
            d->dstFile->reset();
        }

        if (d->requestAborted) {
            emit sigFailed();
            return;
        }

        if (d->reply->error()) {
            QFile::remove(file.absoluteFilePath());

#ifdef DEBUG_INFO
            qInfo() << QString("Download failed: %1.")
                       .arg(d->reply->errorString());
#endif
            emit sigFailed();
            return;
        }

        const QVariant redirectionTarget = d->reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        if (!redirectionTarget.isNull()) {
            const QUrl redirectedUrl = d->requestUrl.resolved(redirectionTarget.toUrl());
            d->dstFile = openFileForWrite(file.absoluteFilePath());
            if (!d->dstFile) {
                emit sigFailed();
                return;
            }

            startRequest(redirectedUrl);
            return;
        }

        QFile::rename(d->fileName, d->resultFileName);
        emit sigFinished();

#ifdef DEBUG_INFO
        qInfo() << QString(tr("Downloaded %1 bytes to %2 in %3") .arg(file.size()) .arg(file.fileName(),
                                                                                        QDir::toNativeSeparators(file.absolutePath())));
        qInfo() << "Finished";
#endif
    });

    connect(d->reply, &QIODevice::readyRead, [this](){
        if (d->dstFile)
            d->dstFile->write(d->reply->readAll());
    });

    connect(d->reply, &QNetworkReply::downloadProgress,
            [this](qint64 bytesReceived, qint64 bytesTotal){
        emit sigProgress(bytesReceived, bytesTotal);

#ifdef DEBUG_INFO
        qInfo() << QString::number(bytesReceived/bytesTotal * 100, 'f', 2) << "%    "
            << bytesReceived / (1024 * 1024) << "MB" << "/" << bytesTotal / (1024 * 1024) << "MB";
#endif
    });

#ifdef DEBUG_INFO
    qInfo() << QString(tr("Downloading %1...").arg(url.toString()));
#endif
}

void DownloadUtil::cancel()
{
    d->requestAborted = true;
    d->reply->abort();
}
