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
#ifndef QTCLASSMANAGER_H
#define QTCLASSMANAGER_H

#include "framework/framework_global.h"

#include <QObject>
#include <QMutex>

DPF_BEGIN_NAMESPACE

template<class CT>
        class GC
{
    CT * ins = nullptr;
public:
    explicit GC() = delete;
    explicit GC(CT* instance)
    {
        ins = instance;
    }
    virtual ~GC()
    {
        if(ins) {
            delete ins;
            ins = nullptr;
        }
    }
};

template<class CT = QObject>
class QtClassManager
{
public:

    virtual ~QtClassManager()
    {
        for (auto val : classList.keys()) {
            remove(val);
        }
    }

    virtual bool append(const QString &name, CT *obj, QString *errorString = nullptr)
    {
        if(name.isEmpty()) {
            delete obj;
            if(errorString)
                *errorString = QObject::tr("Failed, Can't append the empty class name");
            return false;
        }

        if (!obj) {
            if (errorString)
                *errorString = QObject::tr("Failed, Can't append the empty class pointer");
            return false;
        }

        auto castPointer = qobject_cast<QObject*>(obj);
        if (!castPointer) {
            if (errorString)
                *errorString = QObject::tr("Failed, Can't append the class pointer not's qobject");
            return false;
        }

        castPointer->setParent(nullptr);

        if (classList[name]) {
            if (errorString)
                *errorString = QObject::tr("Failed, Objects cannot be added repeatedly");
            return false;
        }
        classList.insert(name, obj);
        return true;
    }

    virtual CT *value(const QString &name) const
    {
        auto res = classList[name];
        return res;
    }

    virtual QList<CT*> values() const
    {
        return classList.values();
    }

    virtual QStringList keys() const
    {
        return classList.keys();
    }

    virtual QString key(CT *value) const
    {
        return classList.key(value);
    }

    virtual bool remove(const QString &name)
    {
        auto itera = classList.begin();
        while (itera != classList.end() && !classList.empty()) {
            if (itera.key() == name) {
                delete itera.value();
                itera = classList.erase(itera);
                return true;
            }
            itera ++;
        }
        return false;
    }

protected:
    QHash<QString, CT*> classList;
};

DPF_END_NAMESPACE

#endif // QTCLASSMANAGER_H
