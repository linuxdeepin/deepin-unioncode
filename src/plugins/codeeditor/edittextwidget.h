#ifndef EDITTEXTWIDGET_H
#define EDITTEXTWIDGET_H

#include <QTextEdit>

#include "ScintillaLsp/LspScintillaEdit.h"
#include "common/lsp/protocol.h"

class EditTextWidgetPrivate;
class EditTextWidget : public LspScintillaEdit
{
    Q_OBJECT
    EditTextWidgetPrivate *const d;
public:
    explicit EditTextWidget(QWidget * parent = nullptr);
    QString currentFile();

public slots:
    void setCurrentFile(const QString &filePath, const QString &workspaceFolder);
    void publishDiagnostics(const lsp::Protocol::Diagnostics &diagnostics);
};

#endif // EDITTEXTWIDGET_H
