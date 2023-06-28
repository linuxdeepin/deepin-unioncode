// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUILDERGENERATOR_H
#define BUILDERGENERATOR_H

#include "builderglobals.h"
#include "common/common.h"
#include "services/builder/ioutputparser.h"
#include "services/project/projectinfo.h"

#include <QWidget>
#include <QMenu>

namespace dpfservice {
class BuilderGenerator : public Generator
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

    virtual void appendOutputParser(std::unique_ptr<IOutputParser>& outputParser) {
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
