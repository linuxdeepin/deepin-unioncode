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
#include "commandparser.h"

#include <framework/framework.h>
#include <framework/lifecycle/pluginsetting.h>
#include <QApplication>
#include <QStyleFactory>

static const char *const IID = "org.deepin.plugin.unioncode";
static const char *const CORE_PLUGIN = "plugin-core";
static const char *const CORE_NAME = "libplugin-core.so";

static bool loadPlugins()
{
    dpfCheckTimeBegin();

    auto &&lifeCycle = dpfInstance.lifeCycle();

    // set plugin iid from qt style
    lifeCycle.setPluginIID(IID);

    dpf::PluginSetting *setting = new dpf::PluginSetting;
    lifeCycle.setSettings(setting);

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

    auto corePlugin = lifeCycle.pluginMetaObj(CORE_PLUGIN);
    if (corePlugin.isNull() || !corePlugin->fileName().contains(CORE_NAME)) {
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

void installTranslator(QApplication &a)
{
    QTranslator *translator = new QTranslator();

    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Translations));
    QFile file(CustomPaths::user(CustomPaths::Flags::Configures)
                                     + QDir::separator() + QString("chooselanguage.support"));

    if (!file.exists()) {
       if (file.open(QFile::ReadWrite)) {
           QLocale locale;
           QString fileName = locale.name() + ".qm";
           file.write(fileName.toUtf8());
           file.close();
       }
    }
    if (file.open(QFile::ReadOnly)) {
        QTextStream txtInput(&file);
        QString language = txtInput.readLine();
        file.close();
        translator->load(result + language);
    }
    a.installTranslator(translator);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CommandParser::instance().process();

    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    installTranslator(a);

    dpfInstance.initialize();
    if (!loadPlugins()) {
        qCritical() << "Failed, Load plugins!";
        abort();
    }

    if (CommandParser::instance().isSet("b") || CommandParser::instance().isSet("k")
            || CommandParser::instance().isSet("a")) {
        CommandParser::instance().buildProject();
        return 0;
    }
    return a.exec();
}
