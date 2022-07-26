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
#include "route.h"
#include "setting.h"

#include <iostream>

Route::Route()
    : backends({})
{

}

Route *Route::instance(){
    static Route ins;
    return &ins;
}

Backend *Route::backend(const Route::Head &head)
{
    return backends.value(head);
}

bool Route::removeBackend(const Route::Head &head)
{
    return backends.remove(head);
}

bool Route::saveBackend(const Route::Head &head, Backend *backend)
{
    auto value = backends.value(head);
    if (value) {
        std::cerr << __FUNCTION__ << " Current head saved backend" << "\n"
                  << " workspace: " << head.workspace.toStdString() << "\n"
                  << " language: " << head.language.toStdString() << "\n"
                  << std::endl;
        return false;
    }

    backends.insert(head, backend);
    QObject::connect(backend, &Backend::aboutToClose, [=](){
        backends.key(backend);
    });
    return true;
}

uint qHash(const Route::Head &key, uint seed)
{
    return qHash(key.workspace + key.language, seed);
}

bool operator ==(const Route::Head &t1, const Route::Head &t2)
{
    return t1.language == t2.language
            && t1.workspace == t2.workspace;
}
