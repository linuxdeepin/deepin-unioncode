#ifndef EDITTEXTWIDGET_H
#define EDITTEXTWIDGET_H

#include <QTextEdit>

#include "ScintillaEdit.h"
#include "common/lsp/protocol.h"

class EditTextWidgetPrivate;
class EditTextWidget : public ScintillaEdit
{
    Q_OBJECT
    EditTextWidgetPrivate *const d;
public:
    explicit EditTextWidget(QWidget * parent = nullptr);
    QString currentFile();

public slots:
    void setCurrentFile(const QString &filePath, const QString &workspaceFolder);
    void publishDiagnostics(const lsp::Protocol::Diagnostics &diagnostics);
    void debugMarginClieced(Scintilla::Position position, Scintilla::KeyMod modifiers, int margin);
};

#endif // EDITTEXTWIDGET_H
