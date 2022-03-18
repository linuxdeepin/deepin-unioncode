#include "consolewidget.h"
#include <QDebug>
static ConsoleWidget *ins{nullptr};
ConsoleWidget *ConsoleWidget::instance()
{
    if (!ins)
        ins = new ConsoleWidget;
    return ins;
}

ConsoleWidget::ConsoleWidget(QWidget *parent)
    : QTermWidget(parent)
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

ConsoleWidget::~ConsoleWidget()
{
    qInfo() << __FUNCTION__;
}
