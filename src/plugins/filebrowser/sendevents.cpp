#include "sendevents.h"
#include "framework.h"

void SendEvents::treeViewDoublueClicked(const QString &filePath)
{
    dpf::Event event;
    event.setTopic("FileBrowser");
    event.setData("TreeView.Item.doubleClicked");
    event.setProperty("FilePath", filePath);
    dpf::EventCallProxy::instance().pubEvent(event);
}

void SendEvents::buildProject(const QString &buildSystem,
                              const QString &buildDir,
                              const QString &buildFilePath,
                              const QStringList &buildArgs)
{
    dpf::Event event;
    event.setTopic("FileBrowser");
    event.setData("TreeView.Menu.Build");
    event.setProperty("BuildSystem", buildSystem);
    event.setProperty("BuildDirectories", buildDir);
    event.setProperty("BuildFilePath", buildFilePath);
    event.setProperty("BuildArguments", buildArgs);
    qInfo() << __FUNCTION__ << event;
    dpf::EventCallProxy::instance().pubEvent(event);
}
