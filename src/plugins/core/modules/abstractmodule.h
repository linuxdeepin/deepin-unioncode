// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef ABSTRACTMODULE_H
#define ABSTRACTMODULE_H

#include <QObject>

class Controller;
class AbstractModule : public QObject
{
    Q_OBJECT

public:
    virtual void initialize(Controller *uiController)
    {Q_ASSERT(uiController); this->uiController = uiController;}

protected:
    Controller *uiController = nullptr;
};

#endif // ABSTRACTMODULE_H
