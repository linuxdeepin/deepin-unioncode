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
#include "common/common.h"

#include <framework/framework.h>
#include <QApplication>
#include <QStyleFactory>

/// @brief PLUGIN_INTERFACE 默认插件iid
static const char *const FM_PLUGIN_INTERFACE = "org.deepin.plugin.unioncode";
static const char *const PLUGIN_CORE = "plugin-core";
static const char *const LIB_CORE = "libplugin-core.so";

static bool pluginsLoad()
{
    dpfCheckTimeBegin();

    auto &&lifeCycle = dpfInstance.lifeCycle();

    // set plugin iid from qt style
    lifeCycle.setPluginIID(FM_PLUGIN_INTERFACE);

    QString pluginsPath = CustomPaths::global(CustomPaths::Plugins);
    qInfo() << QString("run application in %0").arg(pluginsPath);
    lifeCycle.setPluginPaths({pluginsPath});

    qInfo() << "Depend library paths:" << QApplication::libraryPaths();
    qInfo() << "Load plugin paths: " << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!lifeCycle.readPlugins()) {
        qCritical() << "Failed, not found any plugin!";
        return false;
    }

    // 手动初始化Core插件
    auto corePlugin = lifeCycle.pluginMetaObj(PLUGIN_CORE);
    if (corePlugin.isNull() || !corePlugin->fileName().contains(LIB_CORE)) {
        qCritical() << "Failed, not found core plugin!";
        return false;
    }

    if (!lifeCycle.loadPlugin(corePlugin)) {
        qCritical() << "Failed, Load core plugin";
        return false;
    }

    // load plugins without core
    if (!lifeCycle.loadPlugins()) {
        qCritical() << "Failed, Load other plugin error!";
        return false;
    }

    dpfCheckTimeEnd();

    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));
    dpfInstance.initialize();
    QTranslator translator;

    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Translations));
    QFile file(CustomPaths::user(CustomPaths::Flags::Configures)
                                     + QDir::separator() + QString("chooselanguage.support"));
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QTextStream txtInput(&file);
        QString language = txtInput.readLine();
        translator.load(result + language);
        a.installTranslator(&translator);
    } else {
        translator.load(result + "zh_CN.qm");
        a.installTranslator(&translator);
    }

    if (!pluginsLoad()) {
        qCritical() << "Failed, Load plugins!";
        abort();
    }

    return a.exec();
}
