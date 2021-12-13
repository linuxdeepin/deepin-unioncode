#ifndef DETAILSBUTTON_H
#define DETAILSBUTTON_H

#include <QAbstractButton>

class DetailsButtonPrivate;
class DetailsButton : public QAbstractButton
{
    Q_OBJECT
    class DetailsButtonPrivate *const d;
public:
    explicit DetailsButton(QWidget *parent = nullptr);
    virtual ~DetailsButton() override;

protected:
    void paintEvent(QPaintEvent *e) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

#endif // DETAILSBUTTON_H
