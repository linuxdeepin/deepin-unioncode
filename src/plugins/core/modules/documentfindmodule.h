// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCUMENTFINDMODULE_H
#define DOCUMENTFINDMODULE_H

#include "abstractmodule.h"

class FindToolBar;
class DocumentFindModule : public AbstractModule
{
public:
    explicit DocumentFindModule();
    ~DocumentFindModule();

    virtual void initialize(Controller *uiController) override;

private:
    FindToolBar *findToolBar { nullptr };
};

#endif   // DOCUMENTFINDMODULE_H
