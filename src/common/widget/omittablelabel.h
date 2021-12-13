#ifndef OMITTABLELABEL_H
#define OMITTABLELABEL_H

#include <QLabel>
class OmittableLabelPrivate;
class OmittableLabel : public QLabel
{
    OmittableLabelPrivate *const d;
public:
    explicit OmittableLabel(QWidget * parent = nullptr);
    virtual ~OmittableLabel() override;
    void setTextElideMode(Qt::TextElideMode mode = Qt::ElideRight);

protected:
    void paintEvent(QPaintEvent *event) override;

public Q_SLOTS:
    void setText(const QString &);
    QString text();
};

#endif // OMITTABLELABEL_H
