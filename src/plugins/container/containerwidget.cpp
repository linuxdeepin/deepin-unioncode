#include "containerwidget.h"
#include "common/common.h"
#include <QDebug>
static ContainerWidget *ins{nullptr};
ContainerWidget *ContainerWidget::instance()
{
    if (!ins)
        ins = new ContainerWidget;
    return ins;
}

ContainerWidget::ContainerWidget(QWidget *parent)
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
    auto containerUp = CustomPaths::global(CustomPaths::Scripts) + QDir::separator() + QString{"container-up.sh\n"};
    sendText(containerUp);
}

ContainerWidget::~ContainerWidget()
{
    qInfo() << __FUNCTION__;
    sendText("container-down.sh\n");
}
