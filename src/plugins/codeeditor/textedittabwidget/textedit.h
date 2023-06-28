// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>
#include "scintillaeditextern.h"

class StyleLsp;
class StyleSci;
class StyleJsonFile;
class TextEditPrivate;
class TextEdit : public ScintillaEditExtern
{
    Q_OBJECT
    TextEditPrivate *const d;
public:
    explicit TextEdit(QWidget * parent = nullptr);
    virtual ~TextEdit();
    virtual void setFile(const QString &filePath) override;
    virtual StyleLsp *getStyleLsp() const {return nullptr;}
    virtual StyleSci *getStyleSci() const {return nullptr;}
    virtual StyleJsonFile *getStyleFile() const {return nullptr;}

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

#endif // TEXTEDIT_H
