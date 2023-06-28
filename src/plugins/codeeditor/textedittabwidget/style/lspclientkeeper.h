// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
