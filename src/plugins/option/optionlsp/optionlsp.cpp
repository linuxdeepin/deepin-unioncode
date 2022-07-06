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
#include "optionlsp.h"
#include "mainframe/optionlspgenerator.h"

#include "base/abstractmenu.h"
#include "base/abstractmainwindow.h"
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
