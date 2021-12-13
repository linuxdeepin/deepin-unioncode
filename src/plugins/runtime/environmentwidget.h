#ifndef ENVIRONMENTWIDGET_H
#define ENVIRONMENTWIDGET_H

#include <QTableView>

class EnvironmentWidgetPrivate;
class EnvironmentWidget : public QWidget
{
    Q_OBJECT
    EnvironmentWidgetPrivate *const d;
public:
    explicit EnvironmentWidget(QWidget *parent = nullptr);
    virtual ~EnvironmentWidget();
};

#endif // ENVIRONMENTWIDGET_H
