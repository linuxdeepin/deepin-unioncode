// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
