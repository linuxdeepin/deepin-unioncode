/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<huangyub@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#ifndef BUILDERPLUGIN_H
#define BUILDERPLUGIN_H

#include <framework/framework.h>

class Project;
namespace dpfservice {
class WindowService;
}
class BuilderCore : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "buildercore.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

public slots:
    void slotBuildStarted();
    void slotProjectTreeMenu(const QString &program, const QStringList &arguments);

private:
    dpfservice::WindowService *windowService = nullptr;
};

#endif // BUILDERPLUGIN_H
