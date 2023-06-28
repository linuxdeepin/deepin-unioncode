// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dapproxy.h"

DapProxy::DapProxy(QObject *parent)
    : QObject(parent)
{

}

DapProxy::~DapProxy()
{

}

DapProxy *DapProxy::instance()
{
    static DapProxy instance;
    return &instance;
}
