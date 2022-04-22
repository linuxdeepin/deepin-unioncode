#ifndef VIRTUALTABWIDGET_H
#define VIRTUALTABWIDGET_H

#include <QTabWidget>

class VirtualTabWidget : QTabWidget
{
    Q_OBJECT
public:
    VirtualTabWidget();
    virtual ~VirtualTabWidget();

    QTabWidget *getTabWidget();
private:
    QTabWidget *tabWidget;
};

#endif // VIRTUALTABWIDGET_H
