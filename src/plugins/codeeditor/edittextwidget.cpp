#include "edittextwidget.h"
#include "filelangdatabase.h"
#include "sendevents.h"
#include "Document.h"
#include "Lexilla.h"
#include "config.h" //cmake build generate
#include "Lexilla.h"

#include <QDir>
#include <QDebug>
#include <QLibrary>
#include <QApplication>
#include <QTemporaryFile>

Sci_Position getSciPosition(sptr_t doc, const lsp::Protocol::Position &pos)
{
    auto docTemp = (Scintilla::Internal::Document*)(doc);
    return docTemp->GetRelativePosition(docTemp->LineStart(pos.line), pos.character);
}

bool isRuninInstalled()
{
    return QApplication::applicationDirPath() == RUNTIME_INSTALL_PATH;
}

QString lexillaFileName()
{
    return QString(LEXILLA_LIB) + LEXILLA_EXTENSION;
}

QString lexillaFilePath()
{
    if (isRuninInstalled())
        return QString(LEXILLA_INSTALL_PATH) + QDir::separator() + lexillaFileName();
    else
        return QString(LEXILLA_BUILD_PATH)  + QDir::separator() + lexillaFileName();
}

sptr_t createLexerFromLib(const char *LanguageID)
{
    QFileInfo info(lexillaFilePath());
    if (!info.exists()) {
        qCritical() << "Failed, can't found lexilla library: " << info.filePath();
        abort();
    }

    static QLibrary lexillaLibrary(info.filePath());
    if (!lexillaLibrary.isLoaded()) {
        if (!lexillaLibrary.load()) {
            qCritical() << "Failed, to loading lexilla library: "
                        << info.filePath()
                        << lexillaLibrary.errorString();
            abort();
        }
        qInfo() << "Successful, Loaded lexilla library:" << info.filePath()
                << "\nand lexilla library support language count:"
                << ((Lexilla::GetLexerCountFn)(lexillaLibrary.resolve(LEXILLA_GETLEXERCOUNT)))();
    }
    QFunctionPointer fn = lexillaLibrary.resolve(LEXILLA_CREATELEXER);
    if (!fn) {
        qCritical() << lexillaLibrary.errorString();
        abort();
    }
    void *lexCpp = ((Lexilla::CreateLexerFn)fn)(LanguageID);
    return sptr_t(lexCpp);
}

class EditTextWidgetPrivate
{
    friend class EditTextWidget;

    inline int scintillaColor(const QColor &col)
    {
        return (col.blue() << 16) | (col.green() << 8) | col.red();
    }

    QString file = "";
    lsp::Client *client = nullptr;
};

enum
{
    IndicDiagnosticUnkown = 0,
    IndicDiagnosticError = 1,
    IndicDiagnosticWarning = 2,
    IndicDiagnosticInfo = 3,
    IndicDiagnosticHint = 4,
};

EditTextWidget::EditTextWidget(QWidget *parent)
    : ScintillaEdit (parent)
    , d(new EditTextWidgetPrivate)
{
    if (!d->client)
        d->client = new lsp::Client();

    d->client->setProgram("clangd-7");
    d->client->start();

    //    d->client->hoverRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp",
    //                            lsp::Protocol::Position{10,0});
    //    d->client->signatureHelpRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp",
    //                                    lsp::Protocol::Position{10,0});
    //    d->client->completionRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp",
    //                                 lsp::Protocol::Position{10,0});
    //    d->client->definitionRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp",
    //                                 lsp::Protocol::Position{10,0});
    //    d->client->symbolRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp");
    //    d->client->referencesRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp",
    //                                 lsp::Protocol::Position{10,0});
    //    d->client->highlightRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp",
    //                                lsp::Protocol::Position{10,0});
    //    d->client->closeRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp");

    //    d->client->shutdownRequest();
    //    d->client->exitRequest();

    QObject::connect(d->client, QOverload<const lsp::Protocol::Diagnostics &>::of(&lsp::Client::notification),
                     this, &EditTextWidget::publishDiagnostics);

    QObject::connect(this, &EditTextWidget::marginClicked, this, &EditTextWidget::debugMarginClieced);

    setMargins(SC_MAX_MARGIN);

    setMarginWidthN(0, 50);
    setMarginSensitiveN(0, SCN_MARGINCLICK);
    setMarginTypeN(0, SC_MARGIN_SYMBOL);
    setMarginMaskN(0, 0x01);    //range mark 1~32
    markerSetFore(0, 0x0000ff); //red
    markerSetBack(0, 0x0000ff); //red
    markerSetAlpha(0, INDIC_GRADIENT);

    setMarginWidthN(1, 20);
    setMarginTypeN(1, SC_MARGIN_NUMBER);
    setMarginMaskN(1, 0x00);   // null
    markerSetFore(1, 0x0000ff); //red
    markerSetBack(1, 0x0000ff); //red
    markerSetAlpha(1, INDIC_GRADIENT);

    indicSetStyle(0, INDIC_HIDDEN);
    indicSetStyle(1, INDIC_SQUIGGLE);
    indicSetStyle(2, INDIC_SQUIGGLE);
    indicSetStyle(3, INDIC_PLAIN);
    indicSetStyle(4, INDIC_PLAIN);

    indicSetFore(0, 0x000000);
    indicSetFore(1, 0x0000ff);
    indicSetFore(2, 0x0000ff);
    indicSetFore(3, 0x00ffff);
    indicSetFore(4, 0x00ffff);

    styleSetFore(0, d->scintillaColor(QColor(0,0,0))); // 空格
    styleSetFore(2, d->scintillaColor(QColor(0x880000))); // //注释
    styleSetFore(5, d->scintillaColor(QColor(0x008800)));
    styleSetFore(6, d->scintillaColor(QColor(0x004400))); // 字符串
    styleSetFore(9, d->scintillaColor(QColor(0x008888))); // #
    styleSetFore(10, d->scintillaColor(QColor(0xFF00FF))); // 符号
    styleSetFore(11, d->scintillaColor(QColor(0x8800FF))); // 其他关键字
    styleSetFore(15, d->scintillaColor(QColor(0x00FF88))); // ///注释
    styleSetFore(18, d->scintillaColor(QColor(0xFF0088))); // /// @

    //    //
    //    //	Handle autocompletion start
    //    //
    //    connect(this, &LspScintillaEdit::lspCompletion, [=, &autoCompletion](const Scintilla::LspCompletionList &cl) {
    //        autoCompletion = cl;
    //        if (cl.items.size() == 0)
    //        {
    //            autoCCancel();
    //            return;
    //        }
    //        QStringList items;
    //        for (auto i : cl.items)
    //            items.append(QString::fromStdString(i.label).simplified());
    //        autoCSetSeparator('\t');
    //        autoCShow(0, items.join("\t").toUtf8().constData());
    //    });
    //    //
    //    //	Handle autocompletion apply
    //    //
    //    connect(this, &ScintillaEdit::autoCompleteSelection,
    //            [=, &autoCompletion](int position, const QString &text, QChar fillupChar, int listCompletionMethod) {
    //        Q_UNUSED(position)
    //        Q_UNUSED(text)
    //        Q_UNUSED(fillupChar)
    //        Q_UNUSED(listCompletionMethod)
    //        // Prendo l'indice selezionato e cancello la lista
    //        const int curIdx = autoCCurrent();
    //        if (curIdx < 0 || curIdx >= autoCompletion.items.size())
    //            return;
    //        autoCCancel();
    //        // Eseguo l'autocompletamento
    //        const auto &textEdit = autoCompletion.items.at(curIdx).textEdit;
    //        Scintilla::LspScintillaDoc doc(docPointer());
    //        Sci_Position p_start, p_end;
    //        if (!Scintilla::lspConv::convertRange(doc, textEdit.range, p_start, p_end))
    //            return;
    //        setSel(p_start, p_end);
    //        replaceSel(textEdit.newText.c_str());
    //    });
    //    //
    //    //	Handle signature help
    //    //
    //    connect(this, &LspScintillaEdit::lspSignatureHelp, [=](const Scintilla::LspSignatureHelp &help) {
    //        const std::size_t sz = help.signatures.size();
    //        if (!hasFocus() || (sz == 0))
    //            return;
    //        QByteArray text;
    //        Sci_Position hlt_start = 0, hlt_end = 0;
    //        if (sz == 1)
    //            text = QByteArray::fromStdString(help.signatures[0].label);
    //        else for (int i = 0; i < sz; i++)
    //        {
    //            const int idx = (help.activeSignature + i) % sz;
    //            if (!text.isEmpty()) text.append('\n');
    //            text.append("\001[");
    //            text.append(QByteArray::number(idx + 1));
    //            text.append("]\002 ");
    //            if (idx == help.activeSignature)
    //            {
    //                const auto &par = help.signatures[idx].parameters[help.activeParameter];
    //                hlt_start = text.size() + par.label_start;
    //                hlt_end = hlt_start + (par.label_end - par.label_start);
    //            }
    //            text.append(help.signatures[idx].label.c_str());
    //        }
    //        callTipShow(currentPos(), text.constData());
    //        callTipSetHlt(hlt_start, hlt_end);
    //    });
    //
}

QString EditTextWidget::currentFile()
{
    return d->file;
}

void EditTextWidget::setCurrentFile(const QString &filePath, const QString &workspaceFolder)
{
    if (d->file == filePath)
        return;
    QString text;
    QFile file(filePath);
    if (file.open(QFile::OpenModeFlag::ReadOnly)) {
        text = file.readAll();
        file.close();
    }
    setText(text.toUtf8());

    if (!lexer()) {
        setILexer(createLexerFromLib("cpp"));
    }

    d->client->initRequest(workspaceFolder);
    d->client->openRequest(filePath);
    d->client->docHighlightRequest(filePath, lsp::Protocol::Position{0, 0});
}

void EditTextWidget::publishDiagnostics(const lsp::Protocol::Diagnostics &diagnostics)
{
    const auto docLen = length();
    indicatorClearRange(0, docLen);
    for (auto val : diagnostics) {
        Sci_Position startPos = getSciPosition(docPointer(), val.range.start);
        Sci_Position endPos = getSciPosition(docPointer(), val.range.end);
        setIndicatorCurrent(val.severity);
        indicatorFillRange(startPos, endPos - startPos);
    }
}

void EditTextWidget::debugMarginClieced(Scintilla::Position position, Scintilla::KeyMod modifiers, int margin)
{
    Q_UNUSED(modifiers);
    sptr_t line = lineFromPosition(position);
    if (markerGet(line)) {
        SendEvents::marginDebugPointRemove(this->currentFile(), line);
        markerDelete(line, margin);
    } else {
        SendEvents::marginDebugPointAdd(this->currentFile(), line);
        markerAdd(line, 0);
    }
}
