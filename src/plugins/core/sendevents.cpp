#include "sendevents.h"

#include <framework/framework.h>

void SendEvents::navRecentShow()
{
    dpf::Event showNavRecent;
    showNavRecent.setTopic("Nav");
    showNavRecent.setData("Recent.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavRecent);
    qInfo() << __FUNCTION__;
}

void SendEvents::navRecentHide()
{
    dpf::Event hideNavRecent;
    hideNavRecent.setTopic("Nav");
    hideNavRecent.setData("Recent.Hide");
    dpf::EventCallProxy::instance().pubEvent(hideNavRecent);
    qInfo() << __FUNCTION__;
}

void SendEvents::navEditShow()
{
    dpf::Event showNavEdit;
    showNavEdit.setTopic("Nav");
    showNavEdit.setData("Edit.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavEdit);
    qInfo() << __FUNCTION__;
}

void SendEvents::navEditHide()
{
    dpf::Event hideNavEdit;
    hideNavEdit.setTopic("Nav");
    hideNavEdit.setData("Edit.Hide");
    dpf::EventCallProxy::instance().pubEvent(hideNavEdit);
    qInfo() << __FUNCTION__;
}

void SendEvents::navDebugShow()
{
    dpf::Event showNavDebug;
    showNavDebug.setTopic("Nav");
    showNavDebug.setData("Debug.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavDebug);
    qInfo() << __FUNCTION__;
}

void SendEvents::NavDebugHide()
{
    dpf::Event hideNavDebug;
    hideNavDebug.setTopic("Nav");
    hideNavDebug.setData("Debug.Hide");
    dpf::EventCallProxy::instance().pubEvent(hideNavDebug);
    qInfo() << __FUNCTION__;
}

void SendEvents::navRuntimeShow()
{
    dpf::Event showNavRuntime;
    showNavRuntime.setTopic("Nav");
    showNavRuntime.setData("Runtime.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavRuntime);
    qInfo() << __FUNCTION__;
}

void SendEvents::navRuntimeHide()
{
    dpf::Event hideNavRuntime;
    hideNavRuntime.setTopic("Nav");
    hideNavRuntime.setData("Runtime.Hide");
    dpf::EventCallProxy::instance().pubEvent(hideNavRuntime);
    qInfo() << __FUNCTION__;
}

void SendEvents::menuOpenFile(const QString &path)
{
    dpf::Event menuOpenFile;
    menuOpenFile.setTopic("Menu");
    menuOpenFile.setData("File.OpenFile");
    menuOpenFile.setProperty("Path", path);
    dpf::EventCallProxy::instance().pubEvent(menuOpenFile);
    qInfo() << __FUNCTION__;
}

void SendEvents::menuOpenDirectory(const QString &path)
{
    dpf::Event menuOpenFile;
    menuOpenFile.setTopic("Menu");
    menuOpenFile.setData("File.OpenDirectory");
    menuOpenFile.setProperty("Path", path);
    dpf::EventCallProxy::instance().pubEvent(menuOpenFile);
    qInfo() << __FUNCTION__;
}
