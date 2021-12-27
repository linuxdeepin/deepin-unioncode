#include "config.h"
#include <framework/framework.h>
#include <QApplication>

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

    // cmake out definitions "DFM_PLUGIN_PATH" and "DFM_BUILD_OUT_PLGUN_DIR"
    if (QApplication::applicationDirPath() == RUNTIME_INSTALL_PATH) {
        // run dde-file-manager path is /usr/bin, use system install plugins
        qInfo() << QString("run application in %0, load system plugin").arg(RUNTIME_INSTALL_PATH);
        lifeCycle.setPluginPaths({PLUGIN_INSTALL_PATH});
        qApp->setLibraryPaths(qApp->libraryPaths() += PLUGIN_INSTALL_PATH);
    } else {
        // if debug and any read from cmake out build path
        qInfo() << "run application not /usr/bin, load debug plugin";
        lifeCycle.setPluginPaths({PLUGIN_BUILD_PATH});
        qApp->setLibraryPaths(qApp->libraryPaths() += PLUGIN_BUILD_PATH);
    }

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

    dpfInstance.initialize();

    if (!pluginsLoad()) {
        qCritical() << "Failed, Load plugins!";
        abort();
    }

    return a.exec();
}
