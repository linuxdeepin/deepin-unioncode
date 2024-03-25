// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/common.h"

#include <framework/framework.h>
#include <framework/lifecycle/pluginsetting.h>

#include <DApplication>

#include <QApplication>
#include <QStyleFactory>

DWIDGET_USE_NAMESPACE

static const char *const IID = "org.deepin.plugin.unioncode";
static const char *const CORE_PLUGIN = "core";
static const char *const CORE_NAME = "libcore.so";
static bool loadPlugins()
{
    dpfCheckTimeBegin();

    auto &&lifeCycle = dpfInstance.lifeCycle();

    // set plugin iid from qt style
    lifeCycle.setPluginIID(IID);

    dpf::PluginSetting *setting = new dpf::PluginSetting(qApp);
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
    lifeCycle.loadPlugins();

    dpfCheckTimeEnd();

    return true;
}

void installTranslator(DApplication &a)
{
    QTranslator *translator = new QTranslator(&a);

    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Translations));
    QFile file(CustomPaths::user(CustomPaths::Flags::Configures)
                                     + QDir::separator() + QString("chooselanguage.support"));

    if (!file.exists()) {
       if (file.open(QFile::ReadWrite)) {
           QLocale locale;
           QString fileName = locale.name() + ".qm";
           a.loadTranslator(QList<QLocale>() << locale.system());
           file.write(fileName.toUtf8());
           file.close();
       }
    }
    if (file.open(QFile::ReadOnly)) {
        QTextStream txtInput(&file);
        QString language = txtInput.readLine();
        QString name = language.left(language.indexOf("."));
        a.loadTranslator(QList<QLocale>() << QLocale(name));
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
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    DApplication a(argc, argv);
    installTranslator(a);

    QString buildDateInfo = a.translate("Application", "<br/>Built on %1 %2 in %3<br/>")
            .arg(QLatin1String(__DATE__), QLatin1String(__TIME__), ProcessUtil::localPlatform());
    a.setOrganizationName("deepin");
    a.setApplicationDisplayName(a.translate("Application", "deepin-unioncode"));
    a.setApplicationVersion(version());
    a.setProductIcon(QIcon::fromTheme("about_logo"));
    a.setApplicationDescription(a.translate("Application",
                                            "Deepin Union Code is a lightweight integrated development environment,\
                                            featured with multilingual and cross platform compatibility."
                                            ));
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

    dpfInstance.initialize();
    dpfInstance.enbaleBackTrace();

    if (!loadPlugins()) {
        qCritical() << "Failed, Load plugins!";
        abort();
    }

    return a.exec();
}
