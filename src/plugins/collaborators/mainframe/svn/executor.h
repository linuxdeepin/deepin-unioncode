/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "loggindialog.h"

#include "common/common.h"

#include <functional>
#include <memory>

#include <QObject>
#include <QString>

class SvnGenerator : public Generator
{
    Q_OBJECT
public:
    explicit SvnGenerator(bool autoDelete = true);
    void start(const QString &cmd, const QStringList &args);

Q_SIGNALS:
    void message(const QByteArray &array);

private:
    QProcess process;
    bool autoDeleteFlag = true;
};

class SvnExecutor
{
public:
    static SvnExecutor *global()
    {
        static SvnExecutor ins("Svn");
        return &ins;
    }
    SvnExecutor(const QString &cvsName) : svcName(cvsName) {}
    QString name(){return cacheName;}
    QString passwd(){return cachePasswd;}
    QString repos(){return cacheRepos;}
    void setName(const QString &name){cacheName = name;}
    void setPasswd(const QString &passwd){cachePasswd = passwd;}
    void setReposPath(const QString &repos){cacheRepos = repos;}
    virtual bool loggin(const QString &name = "", const QString &passwd = "")
    {
        LogginDialog dialog;
        dialog.setTitleText("loggin user from svn");
        dialog.setName(name);
        dialog.setPasswd(passwd);
        dialog.exec();
        cachePasswd = dialog.passwd();
        cacheName = dialog.name();
        QString loginInfo = QString("--username %0 --passwd %1").arg(cacheName).arg(cachePasswd);
        QString reposOption = QString("list %0").arg(cacheRepos);
        SvnGenerator login;
        login.start("svn", {reposOption, loginInfo});
        return false;
    }

protected:
    QString cacheName;
    QString cachePasswd;
    QString cacheRepos;
    QString svcName;
};


#endif // EXECUTOR_H
