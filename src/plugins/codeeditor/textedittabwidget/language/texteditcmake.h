#ifndef TEXTEDITCMAKE_H
#define TEXTEDITCMAKE_H

#include "textedittabwidget/textedit.h"

class TextEditCmakePrivate;
class TextEditCmake : public TextEdit
{
    Q_OBJECT
    TextEditCmakePrivate *const d;
public:
    explicit TextEditCmake(QWidget * parent = nullptr);
    virtual ~TextEditCmake();
    virtual QString supportLanguage();
    static QString implLanguage();

    virtual StyleLsp *getStyleLsp() const;
    virtual StyleSci *getStyleSci() const;
    virtual StyleJsonFile *getStyleFile() const;
};

#endif // TEXTEDITCMAKE_H
