#ifndef ABSTRACTCONSOLE_H
#define ABSTRACTCONSOLE_H

class AbstractConsolePrivate;
class AbstractConsole
{
    AbstractConsolePrivate *const d;
public:
    AbstractConsole(void *qWidget);
    virtual ~AbstractConsole();
    void* qWidget();
};

#endif // ABSTRACTCONSOLE_H
