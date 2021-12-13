#ifndef CONFIGUREWIDGET_H
#define CONFIGUREWIDGET_H

#include <QScrollArea>

class CollapseWidget;
class ConfigureWidgetPrivate;
class ConfigureWidget : public QScrollArea
{
    Q_OBJECT
    ConfigureWidgetPrivate *const d;
public:
    explicit ConfigureWidget(QWidget *parent = nullptr);
    virtual ~ConfigureWidget();
    void addCollapseWidget(CollapseWidget *widget);

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // CONFIGUREWIDGET_H
