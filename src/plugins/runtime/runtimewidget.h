#ifndef RUNTIMEWIDGET_H
#define RUNTIMEWIDGET_H

#include <QWidget>
#include <QSplitter>

class RuntimeWidgetPrivate;
class RuntimeWidget : public QSplitter
{
    Q_OBJECT
    RuntimeWidgetPrivate *const d;
public:
    explicit RuntimeWidget(QWidget *parent = nullptr);
    virtual ~RuntimeWidget();

signals:

public slots:
};

#endif // RUNTIMEWIDGET_H
