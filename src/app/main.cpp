// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/common.h"

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

void voidMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(type);
    Q_UNUSED(context);
    Q_UNUSED(msg);
    // not ouput qt log when in command mode.
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    CommandParser::instance().process();

    // TODO(Any): put to command processor
    if (CommandParser::instance().isBuildModel()) {
        CommandParser::instance().setModel(CommandParser::CommandLine);
        qInstallMessageHandler(voidMessageOutput);
        if (!loadPlugins()) {
            qCritical() << "Failed, Load plugins!";
            abort();
        }
        commandLine.build();
        return 0;
    }

    installTranslator(a);
    dpfInstance.initialize();
    if (!loadPlugins()) {
        qCritical() << "Failed, Load plugins!";
        abort();
    }

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    return a.exec();
}
