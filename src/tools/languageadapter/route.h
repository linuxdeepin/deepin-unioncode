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
#ifndef ROUTE_H
#define ROUTE_H

#include "handler.h"

#include <QHash>
#include <QTcpSocket>

class Route
{
public:
    Route();

    struct Head
    {
        QString workspace;
        QString language;
        bool isEmpty() const {
            return workspace.isEmpty()
                    && language.isEmpty();
        }
    };

    static Route *instance();

    Handler *backend(const Head &head) const;
    Handler *frontend(const Head &head) const;

    bool saveFrontend(const Head &head, Handler *front);
    bool saveBackend(const Head &head, Handler *back);
    bool removeFreeFronted(Handler *front);
    bool removeFreeBackend(Handler *back);
    Route::Head findHead(Handler *front = nullptr,
                         Handler *back = nullptr) const;
    Handler *findFrontend(Handler *backend) const;
    Handler *findBackend(Handler *frontend) const;
    Handler *getFrontend(const Head &head) const;
    Handler *getBackend(const Head &head) const;
    Handler *createBackend(const Head &head) const;

private:
    QHash<Head, QPair<Handler *, Handler *>> handlerHash;
};

uint qHash(const Route::Head &key, uint seed = 0);

bool operator == (const Route::Head &t1, const Route::Head &t2);

#endif // ROUTE_H
