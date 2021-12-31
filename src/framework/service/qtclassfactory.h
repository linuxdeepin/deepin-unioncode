/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

protected:
    QHash<QString, CreateFunc> constructList;
};

DPF_END_NAMESPACE

#endif // QTCLASSFACTORY_H
