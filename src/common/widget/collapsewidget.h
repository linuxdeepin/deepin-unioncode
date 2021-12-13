#ifndef COLLAPSEWIDGET_H
#define COLLAPSEWIDGET_H

#include <QPaintEvent>
#include <QWidget>

class CollapseWidgetPrivate;
class CollapseWidget : public QWidget
{
    CollapseWidgetPrivate *const d;
public:
    explicit CollapseWidget(QWidget *parent = nullptr);
    explicit CollapseWidget(const QString &title, QWidget *widget, QWidget *parent = nullptr);
    virtual ~CollapseWidget() override;

    QWidget *takeWidget();
    void setWidget(QWidget *widget);
    QWidget* widget();

    void setTitle(const QString &title);
    QString title();

    void setCheckable(bool checkable);
    bool isCheckable();

    bool isChecked();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

public slots:
    void setChecked(bool);

private slots:
    void doChecked(bool);
    void resetHeight(const QVariant &);
};

#endif // COLLAPSEWIDGET_H
