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
#ifndef SERVERAPPLICATION_H
#define SERVERAPPLICATION_H

#include "common/lsp/server/servercmdparse.h"
#include "common/lsp/server/stdinjsonrpcparser.h"

#include <QHash>

#include <functional>
#include <iostream>

namespace newlsp {

class ServerApplicationPrivate;
class ServerApplication : public QObject, ServerCmdParse
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
            return operator<<(qListStr.toStdList());
        }

        std::string operator << (const QStringList &qStrList)
        {
            return operator<<(qStrList.toStdList());
        }
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
#define lspServErr lspServApp->err({localDataTimeStr, __FILE__, std::to_string(__LINE__), "Error"})
#define lspServOut lspServApp->err({localDataTimeStr, __FILE__, std::to_string(__LINE__), "Output"})

#endif // SERVERAPPLICATION_H
