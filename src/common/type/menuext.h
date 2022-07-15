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
#ifndef MENUEXT_H
#define MENUEXT_H

#include <QList>
#include <QJsonArray>
#include <vector>

template <class T, class EnumExt_T>
struct __ext_enum
{
public:
    typedef T type_value;
    typedef int type_index;

    static T value(int index) {
        auto baseAddr = (const char*)(EnumExt_T::get());
        return T (*(const T*)(baseAddr + sizeof(T) *index));
    }

    static type_index index(const T &elem)
    {
        for (int i = 0; i < EnumExt_T::count(); i++) {
            if (value(i) == elem)
                return i;
        }
        return -1;
    }

    static int len() {
        return sizeof(EnumExt_T);
    }

    static int count() {
        return len() == 0 ? 0 : len() / sizeof(type_value);
    }

    static auto get(){
        static EnumExt_T instance;
        return &instance;
    }

    static QList<type_index> toQList() {
        QList<type_index> result;
        for (int i = 0; i < EnumExt_T::count(); i++) {
            result << EnumExt_T::value(i);
        }
        return result;
    }

    static bool contains(const type_value &value) {
        for (int i = 0; i < EnumExt_T::count(); i++) {
            if (value == EnumExt_T::value(i))
                return true;
        }
        return false;
    }

    static QJsonArray toArray(){
        QJsonArray result;
        for (int i = 0; i < EnumExt_T::count(); i++) {
            result << EnumExt_T::value(i);
        }
        return result;
    }

    static std::vector<type_value> toStdVector() {
        std::vector<type_value> result;
        for (int i = 0; i < EnumExt_T::count(); i++) {
            result.push_back(EnumExt_T::value(i));
        }
        return result;
    }
};

#define enum_exp public: const type_value
#define enum_def(name, type) class name : public __ext_enum<type, name>
#define enum_type(name) name::type_value

#endif // MENUEXT_H
