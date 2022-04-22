#include "virtualconsole.h"
#include "common/common.h"
#include <QDir>

VirtualConsole::VirtualConsole(QWidget *parent)
    :QTermWidget (parent)
{
    setMargin(0);
    setTerminalOpacity(0);
    setForegroundRole(QPalette::ColorRole::Background);
    setAutoFillBackground(true);
    if (availableColorSchemes().contains("Linux"))
        setColorScheme("Linux");
    if (availableKeyBindings().contains("linux"))
        setKeyBindings("linux");
    setScrollBarPosition(QTermWidget::ScrollBarRight);
    setTerminalSizeHint(false);
    setAutoClose(false);
    changeDir(QDir::homePath());
    sendText("clear\n");
}

VirtualConsole::~VirtualConsole()
{
    qInfo() << __FUNCTION__;
//    sendText("container-down.sh\n");
}
