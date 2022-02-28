#ifndef STYLESCI_H
#define STYLESCI_H

#include "common/common.h"
#include "ScintillaEdit.h"

#include <QObject>
#include <QColor>

class StyleSci : public QObject
{
    Q_OBJECT
public:
    StyleSci(){}

    enum Margin
    {
        LineNumber = 0,
        Runtime = 1,
        CodeFormat = 2,
        Custom = 3,
    };

    enum MarkerNumber
    {
        Debug = 0,
        Running = 1,
        RunningLineBackground = 2,
        Extern = 3
    };

    void setKeyWords(ScintillaEdit &edit);

    virtual void setStyle(ScintillaEdit &edit);
    virtual void setMargin(ScintillaEdit &edit);
    virtual void setLexer(ScintillaEdit &edit);

    virtual QMap<int, QString> keyWords() const;
    virtual int styleOffset() const;
};



#endif // STYLESCI_H
