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
#ifndef LSPCLIENTKEEPER_H
#define LSPCLIENTKEEPER_H

#include "common/common.h"

class LSPClientKeeper final
{
    Q_DISABLE_COPY(LSPClientKeeper)
public:
    LSPClientKeeper();

    virtual ~LSPClientKeeper();

    static LSPClientKeeper *instance();

    newlsp::Client *get(const newlsp::ProjectKey &key);

private:
    QList<newlsp::ProjectKey> projectKeys;
    newlsp::Client* client{nullptr};
};

#endif // LSPCLIENTKEEPER_H
