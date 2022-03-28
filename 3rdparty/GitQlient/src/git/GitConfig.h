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

#include <QObject>
#include <QSharedPointer>
#include <QString>

#include <GitExecResult.h>

class GitBase;

struct GitUserInfo
{
   QString mUserName;
   QString mUserEmail;

   bool isValid() const;
};

class GitConfig : public QObject
{
   Q_OBJECT

signals:
   void signalCloningProgress(QString stepDescription, int value);
   void signalCloningFailure(int error, QString failure);

public:
   explicit GitConfig(QSharedPointer<GitBase> gitBase, QObject *parent = nullptr);

   GitUserInfo getGlobalUserInfo() const;
   void setGlobalUserInfo(const GitUserInfo &info);
   GitExecResult setGlobalData(const QString &key, const QString &value);
   GitUserInfo getLocalUserInfo() const;
   void setLocalUserInfo(const GitUserInfo &info);
   GitExecResult setLocalData(const QString &key, const QString &value);
   GitExecResult clone(const QString &url, const QString &fullPath);
   GitExecResult initRepo(const QString &fullPath);
   GitExecResult getLocalConfig() const;
   GitExecResult getGlobalConfig() const;
   GitExecResult getRemoteForBranch(const QString &branch);
   GitExecResult getGitValue(const QString &key) const;
   QString getServerUrl() const;
   QString getServerHost() const;
   QPair<QString, QString> getCurrentRepoAndOwner() const;

private:
   QSharedPointer<GitBase> mGitBase;
};
