#pragma once

/****************************************************************************************
 ** GitQlient is an application to manage and operate one or several Git repositories. With
 ** GitQlient you will be able to add commits, branches and manage all the options Git provides.
 ** Copyright (C) 2021  Francesc Martinez
 **
 ** LinkedIn: www.linkedin.com/in/cescmm/
 ** Web: www.francescmm.com
 **
 ** This program is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License as published by the Free Software Foundation; either
 ** version 2 of the License, or (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this library; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***************************************************************************************/
#include <CircularPixmap.h>

#include <QString>
#include <QLabel>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QDir>
#include <QPointer>

inline void storeCreatorAvatar(QNetworkAccessManager *manager, QNetworkReply *reply, QLabel *avatar,
                               const QString &fileName)
{
   const auto data = reply->readAll();
   const auto cache = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
   QDir dir(cache);

   if (!dir.exists())
      dir.mkpath(cache);

   const auto path = QString("%1/%2").arg(dir.absolutePath(), fileName);

   if (QFile file(path); file.open(QIODevice::WriteOnly))
   {
      file.write(data);
      file.close();

      QPixmap img(path);

      if (!img.isNull())
      {
         img = img.scaled(50, 50, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

         avatar->setPixmap(img);
      }
   }

   reply->deleteLater();
   manager->deleteLater();
}

inline QPointer<CircularPixmap> createAvatar(const QString &userName, const QString &avatarUrl,
                                             const QSize &avatarSize = QSize(50, 50))
{
   const auto fileName
       = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation), userName);
   QPointer<CircularPixmap> avatar = new CircularPixmap(avatarSize);
   avatar->setObjectName("Avatar");

   if (!QFile(fileName).exists())
   {
      const auto manager = new QNetworkAccessManager();
      QNetworkRequest request;
      request.setUrl(avatarUrl);
      request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);
      const auto reply = manager->get(request);
      QObject::connect(reply, &QNetworkReply::finished, [manager, reply, avatar, userName]() {
         if (avatar)
            storeCreatorAvatar(manager, reply, avatar, userName);
      });
   }
   else
   {
      QPixmap img(fileName);

      if (!img.isNull())
      {
         img = img.scaled(avatarSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

         avatar->setPixmap(img);
      }
   }

   return avatar;
}
