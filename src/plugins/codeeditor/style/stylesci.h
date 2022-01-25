#ifndef STYLESCI_H
#define STYLESCI_H

#include <QObject>

class StyleSci : public QObject
{
    Q_OBJECT
public:
    StyleSci();
    virtual const char *sciKeyWords(int set) {
        Q_UNUSED(set);
        return "";
    }
    virtual void setDefaultStyle() { return; }
    virtual int indicOffset() { return 0; }
};

#endif // STYLESCI_H
