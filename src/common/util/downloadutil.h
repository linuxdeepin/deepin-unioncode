/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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

