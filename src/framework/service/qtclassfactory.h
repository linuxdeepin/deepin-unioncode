// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QTCLASSFACTORY_H
#define QTCLASSFACTORY_H

#include "framework/framework_global.h"

#include <QString>
#include <QObject>

#include <functional>

DPF_BEGIN_NAMESPACE

template<class CT = QObject>
class QtClassFactory
{
    typedef std::function<CT*()> CreateFunc;
public:
    virtual ~QtClassFactory() {}

    template<class T>
    bool regClass(const QString &name, QString *errorString = nullptr)
    {
        if (constructList[name]) {
            if (errorString)
                *errorString = QObject::tr("The current class name has registered "
                                           "the associated construction class");
            return false;
        }

        CreateFunc foo = [=](){
            return dynamic_cast<CT*>(new T());
        };
        constructList.insert(name, foo);
        return true;
    }

    CT* create(const QString &name, QString *errorString = nullptr)
    {
        CreateFunc constantFunc = constructList.value(name);
        if (constantFunc) {
            return constantFunc();
        } else {
            if (errorString)
                *errorString = QObject::tr("Should be call registered 'regClass()' function "
                                           "before create function");
            return nullptr;
        }
    }

    QStringList createKeys()
    {
        return constructList.keys();
    }

protected:
    QMap<QString, CreateFunc> constructList;
};

DPF_END_NAMESPACE

#endif // QTCLASSFACTORY_H
