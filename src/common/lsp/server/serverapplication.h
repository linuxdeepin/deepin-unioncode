// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVERAPPLICATION_H
#define SERVERAPPLICATION_H

#include "common/common_global.h"
#include "common/lsp/server/servercmdparse.h"
#include "common/lsp/server/stdinjsonrpcparser.h"

#include <QHash>

#include <functional>
#include <iostream>

namespace newlsp {

class ServerApplicationPrivate;
class COMMON_EXPORT ServerApplication : public QObject, ServerCmdParse
{
    Q_OBJECT
    ServerApplicationPrivate *const d;
public:
    struct LogFormat {
        std::vector<std::string> perfixs;
        LogFormat(const std::vector<std::string> &perfixs = {}) : perfixs(perfixs) {}
        std::string perfixsString(){
            std::string perfixsString;
            for (auto &&one : perfixs) {
                perfixsString += "[" + one + "]";
            }
            return perfixsString;
        }
    };

    struct LogValue
    {
        template<class T>
        T operator << (const T &t){
            return t;
        }

        std::string operator << (const QString &qStr){
            return qStr.toStdString();
        }

        std::string operator << (const std::list<QString> &listqStr)
        {
            std::string ret = "";
            for (auto qStr: listqStr) {
                if (ret.empty())
                    ret += "(";
                else
                    ret += "," + qStr.toStdString();
            }
            ret += ")";
            return ret;
        }

        std::string operator << (const QList<QString> &qListStr)
        {
            return operator<<(std::list<QString>(qListStr.begin(), qListStr.end()));
        }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
         std::string operator << (const QStringList &qStrList)
         {
             return operator<<(std::list<QString>(qStrList.begin(), qStrList.end()));
         }
#endif
    };

    struct Stderr : LogFormat, LogValue{
        Stderr(const Stderr &) = delete;
        Stderr(const std::vector<std::string> &perfixs = {}) : LogFormat(perfixs)
        { std::cerr << perfixsString(); }
        ~Stderr() {std::cerr << std::endl;}
        template<class T> Stderr& operator << (const T &t) {
            std::cerr << LogValue::operator<<(t) << " ";
            return *this;
        }
    };

    struct Stdout : LogFormat, LogValue{
        Stdout(const Stderr &) = delete;
        Stdout(const std::vector<std::string> &perfixs = {}) : LogFormat(perfixs)
        { std::cout << perfixsString(); }
        ~Stdout() {std::cout << std::endl;}
        template<class T> Stdout& operator << (const T &t) {
            std::cout << LogValue::operator<<(t) << " ";
            return *this;
        }
    };

    ServerApplication(const QCoreApplication &app);
    virtual ~ServerApplication();
    void start();
    static ServerApplication *ins();
    Stderr err(const std::vector<std::string> &perfixs);
    Stdout out(const std::vector<std::string> &perfixs);

    static QString toProtocolString(int id, const QString method, const QJsonObject &params);
    static QString toProtocolString(const QString &method, const QJsonObject &params);
    static QString toProtocolString(const QJsonObject &object);
    static QString localDateTime();

public Q_SLOTS:
    void jsonrpcJsonOutput(const QJsonObject &obj);

private Q_SLOTS:
    void identifyJsonObject(const QJsonObject &obj);

Q_SIGNALS:
    void jsonrpcMethod(int id, const QString &method, const QJsonObject &params);
    void jsonrpcNotification(const QString &method, const QJsonObject &params);
};

} //newlsp

#define localDataTimeStr newlsp::ServerApplication::localDateTime().toStdString()
#define lspServApp newlsp::ServerApplication::ins()
#define lspServErr lspServApp->err({localDataTimeStr, __FILE__, std::to_string(__LINE__), "Server Error"})
#define lspServOut lspServApp->out({localDataTimeStr, __FILE__, std::to_string(__LINE__), "Server output"})

#endif // SERVERAPPLICATION_H
