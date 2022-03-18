#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include "qtermwidget.h" // 3drparty
#include "ColorScheme.h"

#include <QDir>

class ConsoleWidget : public QTermWidget
{
    Q_OBJECT
public:
    static ConsoleWidget *instance();
    explicit ConsoleWidget(QWidget *parent = nullptr);
    virtual ~ConsoleWidget();
};

#endif // CONSOLEWIDGET_H
