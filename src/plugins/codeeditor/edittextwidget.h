#ifndef EDITTEXTWIDGET_H
#define EDITTEXTWIDGET_H

#include <QTextEdit>

#include "ScintillaLsp/LspScintillaEdit.h"

class EditTextWidgetPrivate;
class EditTextWidget : public LspScintillaEdit
{
    Q_OBJECT
    EditTextWidgetPrivate *const d;
public:
    explicit EditTextWidget(QWidget * parent = nullptr);
    QString currentFile();

public slots:
    void setCurrentFile(const QString &filePath);
};

#endif // EDITTEXTWIDGET_H
