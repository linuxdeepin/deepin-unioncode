// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUILDERGENERATOR_H
#define BUILDERGENERATOR_H

#include "builderglobals.h"
#include "common/common.h"
#include "base/abstractoutputparser.h"
#include "services/services_global.h"

#include <QWidget>
#include <QMenu>

namespace dpfservice {
class SERVICE_EXPORT BuilderGenerator : public Generator
{
    Q_OBJECT
public:
    BuilderGenerator(){}
    virtual ~BuilderGenerator(){}

    virtual BuildCommandInfo getMenuCommand(const BuildMenuType buildMenuType, const dpfservice::ProjectInfo &projectInfo) {
        Q_UNUSED(buildMenuType)
        Q_UNUSED(projectInfo)
        return BuildCommandInfo();
    }

    virtual void appendOutputParser(std::unique_ptr<AbstractOutputParser>& outputParser) {
        Q_UNUSED(outputParser)
    }

    virtual bool checkCommandValidity(const BuildCommandInfo &info, QString &retMsg) {
        Q_UNUSED(info)
        Q_UNUSED(retMsg)
        return true;
    }
};

} // namespace dpfservice

#endif // BUILDERGENERATOR_H
