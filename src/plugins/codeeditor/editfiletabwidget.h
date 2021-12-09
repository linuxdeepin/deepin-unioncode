#ifndef EDITFILETABWIDGET_H
#define EDITFILETABWIDGET_H

#include <QTabBar>

class EditFileTabWidget : public QTabBar
{
    Q_OBJECT
public:
    explicit EditFileTabWidget(QWidget *parent = nullptr);
};

#endif // EDITFILETABWIDGET_H
