#ifndef STYLELSP_H
#define STYLELSP_H

#include "common/common.h"

#include <QObject>

class StyleLsp : public QObject
{
    Q_OBJECT
public:
    StyleLsp();
    virtual ~StyleLsp();

    struct Underline
    {
        int colorHex;
        int style;
    };

    virtual QStringList themes() const;
    virtual bool selectTheme(const QString &theme);
    virtual int fontSize(const QVariant &key) const;
    virtual int background(const QVariant &key) const;
    virtual int foreground(const QVariant &key) const;
    virtual Underline underline(const QVariant &key) const;
};

#endif // STYLELSP_H
