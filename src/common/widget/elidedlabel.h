#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QLabel>

class ElidedLabelPrivate;
class ElidedLabel : public QLabel
{
    Q_OBJECT
    ElidedLabelPrivate *const d;

public:
    explicit ElidedLabel(QWidget *parent = nullptr);
    virtual ~ElidedLabel();
    QString text();

public slots:
    void setText(const QString &);
};

#endif // ELIDEDLABEL_H
