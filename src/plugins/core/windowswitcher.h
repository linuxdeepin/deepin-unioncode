#ifndef WINDOWSWITCHER_H
#define WINDOWSWITCHER_H

#include "windowkeeper.h"

#include <framework/framework.h>

#include <QMainWindow>

//框架实例化
class WindowSwitcher : public dpf::EventHandler, dpf::AutoEventHandlerRegister<WindowSwitcher>
{
    Q_OBJECT
public:
    explicit WindowSwitcher(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event& event);
    virtual void navEvent(const dpf::Event& event);
};
#endif // WINDOWSWITCHER_H
