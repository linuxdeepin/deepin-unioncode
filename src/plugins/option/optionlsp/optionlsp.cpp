// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionlsp.h"
#include "mainframe/optionlspgenerator.h"

#include "base/abstractmenu.h"
#include "base/abstractwidget.h"

#include "services/option/optionservice.h"

#include "framework/listener/listener.h"

using namespace dpfservice;

void OptionLsp::initialize()
{
    DPF_USE_NAMESPACE;
}

bool OptionLsp::start()
{
    //    auto &ctx = dpfInstance.serviceContext();
    //    OptionService *optionService = ctx.service<OptionService>(OptionService::name());
    //    if (!optionService) {
    //        qCritical() << "Failed, not found option service!";
    //        abort();
    //    }
    //    optionService->implGenerator<OptionLspGenerator>(OptionLspGenerator::kitName());
    return true;
}

dpf::Plugin::ShutdownFlag OptionLsp::stop()
{
    return Sync;
}
