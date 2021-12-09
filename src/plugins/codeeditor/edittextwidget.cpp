#include "edittextwidget.h"
#include "filelangdatabase.h"
#include "../lsp/lspclient.h"
#include "../lsp/lspstructures.h"
#include "../lsp/ILspDocument.h"

#include <QDebug>
#include <QApplication>
#include <QTemporaryFile>

class EditTextWidgetPrivate
{
    friend class EditTextWidget;

    inline int scintillaColor(const QColor &col)
    {
        return (col.blue() << 16) | (col.green() << 8) | col.red();
    }

    QString file = "";
    Scintilla::LspClient *sessionClient = nullptr;
};

enum
{
    IndicLspDiagnosticHint = INDICATOR_CONTAINER,
    IndicLspDiagnosticInfo = INDICATOR_CONTAINER + 1,
    IndicLspDiagnosticWarning = INDICATOR_CONTAINER + 2,
    IndicLspDiagnosticError = INDICATOR_CONTAINER + 3
};

EditTextWidget::EditTextWidget(QWidget *parent)
    : LspScintillaEdit (parent)
    , d(new EditTextWidgetPrivate)
{
    if (!d->sessionClient)
        d->sessionClient = new Scintilla::LspClient();

    qInfo() << __FUNCTION__ << this->lexerLanguage();

    //
    //	Editor configuration
    //
    styleSetFont(STYLE_DEFAULT, "consolas");
    styleSetSize(STYLE_DEFAULT, 12);
    indicSetStyle(IndicLspDiagnosticError, INDIC_SQUIGGLE);
    indicSetStyle(IndicLspDiagnosticWarning, INDIC_DIAGONAL);
    indicSetStyle(IndicLspDiagnosticInfo, INDIC_TT);
    indicSetStyle(IndicLspDiagnosticHint, INDIC_DOTS);
    // Styles for LSP
    styleSetFore(Scintilla::LspClient::FirstLspStyle, d->scintillaColor(QColor(0xFF0000)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 1, d->scintillaColor(QColor(0x880000)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 2, d->scintillaColor(QColor(0x440000)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 3, d->scintillaColor(QColor(0x00FF00)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 4, d->scintillaColor(QColor(0x008800)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 5, d->scintillaColor(QColor(0x004400)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 6, d->scintillaColor(QColor(0x0000FF)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 7, d->scintillaColor(QColor(0x000088)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 8, d->scintillaColor(QColor(0x008888)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 9, d->scintillaColor(QColor(0xFF00FF)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 10, d->scintillaColor(QColor(0x8800FF)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 11, d->scintillaColor(QColor(0x4400FF)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 12, d->scintillaColor(QColor(0xFF0088)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 13, d->scintillaColor(QColor(0x00FFFF)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 14, d->scintillaColor(QColor(0x00FF88)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 15, d->scintillaColor(QColor(0x00FF44)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 16, d->scintillaColor(QColor(0x0088FF)));
    styleSetFore(Scintilla::LspClient::FirstLspStyle + 17, d->scintillaColor(QColor(0xFF0088)));

    // NOTE: this variable is here for demonstration, it should be part of editor
    Scintilla::LspCompletionList autoCompletion;

    //
    //	Handle autocompletion start
    //
    connect(this, &LspScintillaEdit::lspCompletion, [=, &autoCompletion](const Scintilla::LspCompletionList &cl) {
        autoCompletion = cl;
        if (cl.items.size() == 0)
        {
            autoCCancel();
            return;
        }
        QStringList items;
        for (auto i : cl.items)
            items.append(QString::fromStdString(i.label).simplified());
        autoCSetSeparator('\t');
        autoCShow(0, items.join("\t").toUtf8().constData());
    });
    //
    //	Handle autocompletion apply
    //
    connect(this, &ScintillaEdit::autoCompleteSelection,
            [=, &autoCompletion](int position, const QString &text, QChar fillupChar, int listCompletionMethod) {
        Q_UNUSED(position)
        Q_UNUSED(text)
        Q_UNUSED(fillupChar)
        Q_UNUSED(listCompletionMethod)
        // Prendo l'indice selezionato e cancello la lista
        const int curIdx = autoCCurrent();
        if (curIdx < 0 || curIdx >= autoCompletion.items.size())
            return;
        autoCCancel();
        // Eseguo l'autocompletamento
        const auto &textEdit = autoCompletion.items.at(curIdx).textEdit;
        Scintilla::LspScintillaDoc doc(docPointer());
        Sci_Position p_start, p_end;
        if (!Scintilla::lspConv::convertRange(doc, textEdit.range, p_start, p_end))
            return;
        setSel(p_start, p_end);
        replaceSel(textEdit.newText.c_str());
    });
    //
    //	Handle signature help
    //
    connect(this, &LspScintillaEdit::lspSignatureHelp, [=](const Scintilla::LspSignatureHelp &help) {
        const std::size_t sz = help.signatures.size();
        if (!hasFocus() || (sz == 0))
            return;
        QByteArray text;
        Sci_Position hlt_start = 0, hlt_end = 0;
        if (sz == 1)
            text = QByteArray::fromStdString(help.signatures[0].label);
        else for (int i = 0; i < sz; i++)
        {
            const int idx = (help.activeSignature + i) % sz;
            if (!text.isEmpty()) text.append('\n');
            text.append("\001[");
            text.append(QByteArray::number(idx + 1));
            text.append("]\002 ");
            if (idx == help.activeSignature)
            {
                const auto &par = help.signatures[idx].parameters[help.activeParameter];
                hlt_start = text.size() + par.label_start;
                hlt_end = hlt_start + (par.label_end - par.label_start);
            }
            text.append(help.signatures[idx].label.c_str());
        }
        callTipShow(currentPos(), text.constData());
        callTipSetHlt(hlt_start, hlt_end);
    });
    //
    //	Handle document diagnostics
    //
    connect(this, &LspScintillaEdit::lspDiagnostic, [=](const Scintilla::LspDocumentDiagnostic &dd) {
        const auto docLen = length();
        // Clear indicators used for diagnostics
        setIndicatorCurrent(IndicLspDiagnosticHint);
        indicatorClearRange(0, docLen);
        setIndicatorCurrent(IndicLspDiagnosticInfo);
        indicatorClearRange(0, docLen);
        setIndicatorCurrent(IndicLspDiagnosticWarning);
        indicatorClearRange(0, docLen);
        setIndicatorCurrent(IndicLspDiagnosticError);
        indicatorClearRange(0, docLen);
        const Scintilla::LspScintillaDoc doc(docPointer());
        for (const auto &e : dd)
        {
            const Sci_Position posFrom = Scintilla::lspConv::sciPos(doc, e.range.start);
            const Sci_Position posTo = Scintilla::lspConv::sciPos(doc, e.range.end);
            //
            //	Set indicator
            //
            switch (e.severity)
            {
                case Scintilla::LspDiagnosticElement::Hint:
                    setIndicatorCurrent(IndicLspDiagnosticHint);
                    break;
                case Scintilla::LspDiagnosticElement::Information:
                    setIndicatorCurrent(IndicLspDiagnosticInfo);
                    break;
                case Scintilla::LspDiagnosticElement::Warning:
                    setIndicatorCurrent(IndicLspDiagnosticWarning);
                    break;
                case Scintilla::LspDiagnosticElement::Error:
                    setIndicatorCurrent(IndicLspDiagnosticError);
                    break;
            }
            indicatorFillRange(posFrom, posTo - posFrom);
        }
    });
}

QString EditTextWidget::currentFile()
{
    return d->file;
}

void EditTextWidget::setCurrentFile(const QString &filePath)
{
    if (d->file == filePath)
        return;

    d->file = filePath;
    QUrl uri = QUrl::fromLocalFile(filePath);
    QString supportLanguage = FileLangDatabase::instance().language(filePath);
    qInfo() << "supportLanguage" << supportLanguage;
    d->sessionClient->addDocument(Scintilla::LspScintillaDoc(docPointer()),
                                  uri.toString().toStdString(), "cpp");
    // Initialize and start LSP server
    d->sessionClient->lspStartServer("clangd", "");
    Scintilla::LspClientConfiguration cfg;
    cfg.rootUri = QStringLiteral("file:///%1").arg(QApplication::applicationDirPath()).toStdString();
    // Default not supported for now, handling is buggy
    cfg.semanticHighlighting.supported = true;
    // Add document to LSP server
    d->sessionClient->lspInitialize(cfg);
    setupLspClient(d->sessionClient);
    QFile file(filePath);
    if (file.open(QFile::OpenModeFlag::ReadWrite)) {
        setText(file.readAll().toStdString().c_str());
    }
}

