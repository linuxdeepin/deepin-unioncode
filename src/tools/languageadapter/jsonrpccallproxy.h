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
#ifndef JSONRPCCALLPROXY_H
#define JSONRPCCALLPROXY_H

#include "common/lsp/lsp.h"

#include <QProcess>

/**
 * @brief The JsonRpcCallProxy class
 * only create main thread
 */
class JsonRpcCallProxy : public newlsp::Route
{
    Q_OBJECT
public:
    typedef std::function<QProcess*(const newlsp::ProjectKey&)> ProcCreatFunc;
    typedef std::function<bool (const QJsonObject &, QJsonObject &)> MethodFilterFunc;
    typedef std::function<void (const QJsonObject &)> NotificationFilterFunc;

    JsonRpcCallProxy(const JsonRpcCallProxy &) = delete;
    explicit JsonRpcCallProxy(QObject *parent = nullptr);

    static JsonRpcCallProxy &ins();

    bool bindCreateProc(const std::string &language, const ProcCreatFunc &func);
    QProcess *createLspServ(const newlsp::ProjectKey &key);

    void bindFilter(const QString &methodName, const MethodFilterFunc &func);
    void bindFilter(const QString &methodName, const NotificationFilterFunc &func);

    newlsp::ProjectKey getSelect() { return select; }
    void setSelect(const newlsp::ProjectKey &value) { select = value;}

public slots:
    void methodFilter(int id, const QString &method, const QJsonObject &params);
    void notificationFilter(const QString &method, const QJsonObject &params);

private:
    newlsp::ProjectKey select;
    QHash<QString, MethodFilterFunc> redMethods;
    QHash<QString, NotificationFilterFunc> redNotifications;
    QHash<QString, ProcCreatFunc> createFuncs;
};

#endif // JSONRPCCALLPROXY_H
