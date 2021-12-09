#include "sendevents.h"

#include <framework/framework.h>

void SendEvents::navRecentShow()
{
    qInfo() << __FUNCTION__;
    dpf::Event showNavRecent;
    showNavRecent.setTopic("Nav");
    showNavRecent.setData("Recent.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavRecent);
}

void SendEvents::navRecentHide()
{
    qInfo() << __FUNCTION__;
    dpf::Event hideNavRecent;
    hideNavRecent.setTopic("Nav");
    hideNavRecent.setData("Recent.Hide");
    dpf::EventCallProxy::instance().pubEvent(hideNavRecent);
}

void SendEvents::navEditShow()
{
    qInfo() << __FUNCTION__;
    dpf::Event showNavEdit;
    showNavEdit.setTopic("Nav");
    showNavEdit.setData("Edit.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavEdit);
}

void SendEvents::navEditHide()
{
    qInfo() << __FUNCTION__;
    dpf::Event hideNavEdit;
    hideNavEdit.setTopic("Nav");
    hideNavEdit.setData("Edit.Hide");
    dpf::EventCallProxy::instance().pubEvent(hideNavEdit);
}

void SendEvents::navDebugShow()
{
    qInfo() << __FUNCTION__;
    dpf::Event showNavDebug;
    showNavDebug.setTopic("Nav");
    showNavDebug.setData("Debug.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavDebug);
}

void SendEvents::NavDebugHide()
{
    qInfo() << __FUNCTION__;
    dpf::Event hideNavDebug;
    hideNavDebug.setTopic("Nav");
    hideNavDebug.setData("Debug.Hide");
    dpf::EventCallProxy::instance().pubEvent(hideNavDebug);
}

void SendEvents::navRuntimeShow()
{
    qInfo() << __FUNCTION__;
    dpf::Event showNavRuntime;
    showNavRuntime.setTopic("Nav");
    showNavRuntime.setData("Runtime.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavRuntime);
}

void SendEvents::navRuntimeHide()
{
    qInfo() << __FUNCTION__;
    dpf::Event hideNavRuntime;
    hideNavRuntime.setTopic("Nav");
    hideNavRuntime.setData("Runtime.Hide");
    dpf::EventCallProxy::instance().pubEvent(hideNavRuntime);
}

void SendEvents::menuOpenFile(const QString &filePath)
{
    qInfo() << __FUNCTION__;
    dpf::Event menuOpenFile;
    menuOpenFile.setTopic("Menu");
    menuOpenFile.setData("File.OpenDocument");
    menuOpenFile.setProperty("FilePath", filePath);
    dpf::EventCallProxy::instance().pubEvent(menuOpenFile);
    navEditShow();
}

void SendEvents::menuOpenDirectory(const QString &filePath)
{
    qInfo() << __FUNCTION__;
    dpf::Event menuOpenFile;
    menuOpenFile.setTopic("Menu");
    menuOpenFile.setData("File.OpenFolder");
    menuOpenFile.setProperty("FilePath", filePath);
    dpf::EventCallProxy::instance().pubEvent(menuOpenFile);
    navEditShow();
}
