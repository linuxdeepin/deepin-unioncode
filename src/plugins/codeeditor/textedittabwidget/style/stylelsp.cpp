#include "stylelsp.h"
#include "Scintilla.h"
#include "stylekeeper.h"
#include "stylecolor.h"
#include "textedittabwidget/scintillaeditextern.h"

#include "common/common.h"
#include "framework/service/qtclassmanager.h"

#include "services/workspace/workspaceservice.h"
#include "Document.h"
#include <QHash>
#include <QTimer>

class StyleLspPrivate
{
    QHash<QString, ScintillaEditExtern*> editors;
    Scintilla::Position hoverPos = -1;
    // response not return url, cache send to callback method
    ScintillaEditExtern *currEditorCache = nullptr;
    QString editText = "";
    uint editCount = 0;
    bool checkVersionOk = false;
    friend class StyleLsp;
};

// from ascii code
inline bool StyleLsp::isCharSymbol(const char ch) {
    return (ch >= 0x21 && ch < 0x2F + 1) || (ch >= 0x3A && ch < 0x40 + 1)
            || (ch >= 0x5B && ch < 0x60 + 1) || (ch >= 0x7B && ch < 0x7E + 1);
}

Sci_Position StyleLsp::getSciPosition(sptr_t doc, const lsp::Position &pos)
{
    auto docTemp = (Scintilla::Internal::Document*)(doc);
    return docTemp->GetRelativePosition(docTemp->LineStart(pos.line), pos.character);
}

lsp::Position StyleLsp::getLspPosition(sptr_t doc, sptr_t sciPosition)
{
    auto docTemp = (Scintilla::Internal::Document*)(doc);
    int line = docTemp->LineFromPosition(sciPosition);
    Sci_Position lineChStartPos = getSciPosition(doc, lsp::Position{line, 0});
    return lsp::Position{line, (int)(sciPosition - lineChStartPos)};
}

StyleLsp::StyleLsp()
    : d (new StyleLspPrivate())
{

}

StyleLsp::~StyleLsp()
{
    if (lspClient.state() == QProcess::Starting) {
        lspClient.shutdownRequest();
        lspClient.waitForBytesWritten();
        lspClient.close();
        lspClient.waitForFinished();
    }
}

int StyleLsp::getLspCharacter(sptr_t doc, sptr_t sciPosition)
{
    return getLspPosition(doc, sciPosition).character;
}

void StyleLsp::cleanDiagnostics(ScintillaEdit &edit)
{
    edit.eOLAnnotationClearAll();
    const auto docLen = edit.length();
    edit.indicatorClearRange(0, docLen); // clean all indicator range style
    for (int line = 0; line < edit.lineCount(); line ++) {
        edit.markerDelete(line, Error);
        edit.markerDelete(line, ErrorLineBackground);
        edit.markerDelete(line, Warning);
        edit.markerDelete(line, WarningLineBackground);
        edit.markerDelete(line, Information);
        edit.markerDelete(line, InformationLineBackground);
        edit.markerDelete(line, Hint);
        edit.markerDelete(line, HintLineBackground);
    }
}

void StyleLsp::cleanTokenFull(ScintillaEdit &edit)
{

}

void StyleLsp::cleanCompletion(ScintillaEdit &edit)
{

}

void StyleLsp::cleanHover(ScintillaEdit &edit)
{
    edit.callTipCancel();
}

void StyleLsp::cleanDefinition(ScintillaEdit &edit)
{
    //    auto hoverPos = positionFromPoint(d->definitionPos.x(), d->definitionPos.y());
    //    auto defsStartPos = wordStartPosition(hoverPos, true);
    //    auto defsEndPos = wordEndPosition(hoverPos, true);
    //    indicatorClearRange(defsStartPos, defsEndPos - defsStartPos);
}

void StyleLsp::sciTextInserted(Scintilla::Position position,
                               Scintilla::Position length, Scintilla::Position linesAdded,
                               const QByteArray &text, Scintilla::Position line)
{
    auto edit = qobject_cast<ScintillaEditExtern*>(sender());
    if (!edit)
        return;

    cleanCompletion(*edit);
    cleanDiagnostics(*edit);
    client().changeRequest(edit->file(), edit->textRange(0, edit->length()));

    d->currEditorCache = edit;

    if (length != 1){
        return;
    }

    if (text != " ") {
        d->editCount += length;
        d->editText += text;
        client().completionRequest(edit->file(), getLspPosition(edit->docPointer(), position));
    } else {
        d->editCount = 0;
        d->editText.clear();
        cleanCompletion(*edit);
    }
}

void StyleLsp::sciTextDeleted(Scintilla::Position position,
                              Scintilla::Position length, Scintilla::Position linesAdded,
                              const QByteArray &text, Scintilla::Position line)
{
    auto edit = qobject_cast<ScintillaEditExtern*>(sender());
    if (!edit)
        return;

    qInfo() << "position" << position
            << "length" << length
            << "linesAdded" << linesAdded
            << "text" << text
            << "line" << line;

    cleanCompletion(*edit);
    cleanDiagnostics(*edit);
    client().changeRequest(edit->file(), edit->textRange(0, edit->length()));

    if (length != 1){
        return;
    }

    if (d->editCount > 0) {
        d->editCount -= length;
        if (d->editCount != 0) {
            d->editText.remove(d->editText.count() - 1 , length);
            client().completionRequest(edit->file(), getLspPosition(edit->docPointer(), position));
        } else {
            d->editText.clear();
        }
    } else {
        d->editCount = 0;
        d->editText.clear();
    }
}

void StyleLsp::sciHovered(Scintilla::Position position)
{
    auto edit = qobject_cast<ScintillaEditExtern*>(sender());
    if (!edit)
        return;

    if (edit->isLeave())
        return;

    d->currEditorCache = edit;
    d->hoverPos = position;
    auto lspPostion = getLspPosition(edit->docPointer(), d->hoverPos);
    client().docHoverRequest(edit->file(), lspPostion);
}

void StyleLsp::sciHoverCleaned(Scintilla::Position position)
{
    Q_UNUSED(position);
    auto edit = qobject_cast<ScintillaEditExtern*>(sender());
    if (!edit)
        return;

    cleanHover(*edit);
    d->hoverPos = -1;
    d->currEditorCache = nullptr;
}

lsp::Client &StyleLsp::client()
{
    if (lspClient.program().isEmpty()
            && lspClient.state() == QProcess::ProcessState::NotRunning) {
        support_file::Language::initialize();
        auto serverInfo = support_file::Language::sever(StyleKeeper::key(this));

        // exists language server
        if (!serverInfo.progrma.isEmpty() || ProcessUtil::exists(serverInfo.progrma)) {
            // clang version lower 7
            if (serverInfo.progrma == "clangd") {
                QRegularExpression regExp("[0-9]*\\.[0-9]*\\.[0-9]*");
                auto versionMatchs = regExp.match(ProcessUtil::version(serverInfo.progrma)).capturedTexts();
                for (auto versionMatch : versionMatchs){
                    QStringList versions = versionMatch.split(".");
                    if (versions.size() == 3) {
                        auto major =  versions[0];
                        if (major.toInt() >= 10) { //版本需要大于等于10
                            d->checkVersionOk = true;
                        }
                    }
                }

                if (!d->checkVersionOk) {
#if 0//TODO(any):dialog will pop in loop, fix should be done.
                    ContextDialog::ok(lsp::Client::tr("clangd lower verion: 10, "
                                                     "Does not meet the current operating environment"));
#endif
                    return lspClient;
                }
            }
        }

        lspClient.setProgram(serverInfo.progrma);
        lspClient.setArguments(serverInfo.arguments);
        lspClient.start();
    }

    return lspClient;
}

void StyleLsp::appendEdit(ScintillaEditExtern *editor)
{
    if (!editor) {
        return;
    }

    setIndicStyle(*editor);
    setMargin(*editor);
    d->editors.insert(editor->file(), editor);

    QObject::connect(editor, &ScintillaEditExtern::textInserted, this, &StyleLsp::sciTextInserted);
    QObject::connect(editor, &ScintillaEditExtern::textDeleted, this, &StyleLsp::sciTextDeleted);
    QObject::connect(editor, &ScintillaEditExtern::hovered, this, &StyleLsp::sciHovered);
    QObject::connect(editor, &ScintillaEditExtern::hoverCleaned, this, &StyleLsp::sciHoverCleaned);

    QObject::connect(editor, &ScintillaEditExtern::destroyed, this, [=](QObject *obj){
        auto itera = d->editors.begin();
        while (itera != d->editors.end()) {
            if (itera.value() == obj) {
                client().closeRequest(itera.key());
                d->editors.erase(itera);
                return;
            }
            itera ++;
        }
    });

    //bind signals to file diagnostics
    QObject::connect(&client(), QOverload<const lsp::DiagnosticsParams &>::of(&lsp::Client::notification),
                     this, [=](const lsp::DiagnosticsParams &params){
        auto editor = findSciEdit(params.uri.toLocalFile());
        if (editor) {
            this->cleanDiagnostics(*editor);
            this->setDiagnostics(*editor, params);
        }
    });

    QObject::connect(&client(), QOverload<const QList<lsp::Data>&>::of(&lsp::Client::requestResult),
                     this, [=](const QList<lsp::Data> &data)
    {
        //        this, &EditTextWidget::tokenFullResult, Qt::UniqueConnection);
    });

    QObject::connect(&client(), QOverload<const lsp::SemanticTokensProvider&>::of(&lsp::Client::requestResult),
                     this, [=](const lsp::SemanticTokensProvider& provider){
        //        this, &EditTextWidget::tokenDefinitionsSave, Qt::UniqueConnection);
    });

    QObject::connect(&client(), QOverload<const lsp::Hover&>::of(&lsp::Client::requestResult),
                     this, [=](const lsp::Hover& hover){
        if (!d->currEditorCache) {
            return;
        }
        setHover(*d->currEditorCache, hover);
    });

    QObject::connect(&client(), QOverload<const lsp::CompletionProvider&>::of(&lsp::Client::requestResult),
                     this, [=](const lsp::CompletionProvider& provider){
        if (!d->currEditorCache) {
            return;
        }
        setCompletion(*d->currEditorCache, provider);
        //        this, &EditTextWidget::completionsSave, Qt::UniqueConnection);
    });

    QObject::connect(&client(), QOverload<const lsp::DefinitionProvider&>::of(&lsp::Client::requestResult),
                     this, [=](const lsp::DefinitionProvider&){
        //        this, &EditTextWidget::definitionSave, Qt::UniqueConnection);
    });

    QObject::connect(&client(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [=](int code, auto status) {
        qInfo() << code << status;
    });

    using namespace dpfservice;
    auto &&ctx = dpfInstance.serviceContext();
    auto workspaceService = ctx.service<WorkspaceService>(WorkspaceService::name());
    QString workspaceGenPath;
    if (workspaceService) {
        QStringList workspaceDirs = workspaceService->findWorkspace(editor->file());
        if (workspaceDirs.size() != 1) {
            qCritical() << "Failed, match workspace to much!!!";
        } else {
            workspaceGenPath = workspaceService->targetPath(workspaceDirs[0]);
        }
    }

    client().initRequest(editor->rootPath());
    client().openRequest(editor->file());
}

QString StyleLsp::sciEditFile(ScintillaEditExtern * const sciEdit)
{
    return d->editors.key(sciEdit);
}

void StyleLsp::setIndicStyle(ScintillaEdit &edit)
{
    edit.indicSetStyle(RedSquiggle, INDIC_SQUIGGLE);
    edit.indicSetFore(RedSquiggle, StyleColor::color(StyleColor::Table::get()->Red));

    edit.indicSetStyle(RedTextFore, INDIC_TEXTFORE);
    edit.indicSetFore(RedTextFore, StyleColor::color(StyleColor::Table::get()->Red));
}

void StyleLsp::setMargin(ScintillaEdit &edit)
{
    edit.setMargins(SC_MAX_MARGIN);
    edit.setMarginTypeN(LspCustom, SC_MARGIN_SYMBOL);
    edit.setMarginMaskN(Margin::LspCustom, 1 << LspCustomMarker::Error | 1 << LspCustomMarker::ErrorLineBackground
                        | 1 << LspCustomMarker::Warning | 1 << LspCustomMarker::WarningLineBackground
                        | 1 << LspCustomMarker::Information | 1 << LspCustomMarker::InformationLineBackground
                        | 1 << LspCustomMarker::Hint | 1 << LspCustomMarker::HintLineBackground);

    edit.markerDefine(LspCustomMarker::Error, SC_MARK_ROUNDRECT);
    edit.markerDefine(LspCustomMarker::Warning, SC_MARK_ROUNDRECT);
    edit.markerDefine(LspCustomMarker::Information, SC_MARK_ROUNDRECT);
    edit.markerDefine(LspCustomMarker::Hint, SC_MARK_ROUNDRECT);

    edit.markerDefine(LspCustomMarker::ErrorLineBackground, SC_MARK_BACKGROUND);
    edit.markerDefine(LspCustomMarker::WarningLineBackground, SC_MARK_BACKGROUND);
    edit.markerDefine(LspCustomMarker::InformationLineBackground, SC_MARK_BACKGROUND);
    edit.markerDefine(LspCustomMarker::HintLineBackground, SC_MARK_BACKGROUND);

    edit.markerSetFore(LspCustomMarker::Error, StyleColor::color(StyleColor::Table::get()->Red));
    edit.markerSetBack(LspCustomMarker::Error, StyleColor::color(StyleColor::Table::get()->Red));
    edit.markerSetFore(LspCustomMarker::Warning, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetBack(LspCustomMarker::Warning, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetFore(LspCustomMarker::Information, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetBack(LspCustomMarker::Information, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetFore(LspCustomMarker::Hint, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetBack(LspCustomMarker::Hint, StyleColor::color(StyleColor::Table::get()->Yellow));

    edit.markerSetFore(LspCustomMarker::ErrorLineBackground, StyleColor::color(StyleColor::Table::get()->Red));
    edit.markerSetBack(LspCustomMarker::ErrorLineBackground, StyleColor::color(StyleColor::Table::get()->Red));
    edit.markerSetAlpha(LspCustomMarker::ErrorLineBackground, 0x22);
    edit.markerSetFore(LspCustomMarker::WarningLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetBack(LspCustomMarker::WarningLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetAlpha(LspCustomMarker::WarningLineBackground, 0x55);
    edit.markerSetFore(LspCustomMarker::InformationLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetBack(LspCustomMarker::InformationLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetAlpha(LspCustomMarker::InformationLineBackground, 0x55);
    edit.markerSetFore(LspCustomMarker::HintLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetBack(LspCustomMarker::HintLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetAlpha(LspCustomMarker::HintLineBackground, 0x55);
}

void StyleLsp::setDiagnostics(ScintillaEdit &edit, const lsp::DiagnosticsParams &params)
{
    for (auto val : params.diagnostics) {
        if (val.severity == lsp::Diagnostic::Severity::Error) { // error
            Sci_Position startPos = getSciPosition(edit.docPointer(), val.range.start);
            Sci_Position endPos = getSciPosition(edit.docPointer(), val.range.end);
            edit.setIndicatorCurrent(RedSquiggle);
            edit.indicatorFillRange(startPos, endPos - startPos);

            edit.eOLAnnotationSetText(val.range.start.line,"Error: " + val.message.toLatin1());
            edit.eOLAnnotationSetStyleOffset(EOLAnnotation::RedTextFore);
            edit.eOLAnnotationSetStyle(val.range.start.line, EOLAnnotation::RedTextFore - edit.eOLAnnotationStyleOffset());
            edit.styleSetFore(EOLAnnotation::RedTextFore, StyleColor::color(StyleColor::Table::get()->Red));
            edit.eOLAnnotationSetVisible(EOLANNOTATION_STANDARD);
            edit.markerAdd(val.range.start.line, Error);
            edit.markerAdd(val.range.start.line, ErrorLineBackground);
        }
    }
}

ScintillaEditExtern *StyleLsp::findSciEdit(const QString &file)
{
    return d->editors.value(file);
}

void StyleLsp::setTokenFull(ScintillaEdit &edit, const QList<lsp::Data> &tokens)
{
    if (!edit.lexer())
        return;

    if (StyleKeeper::key(this) != edit.lexerLanguage()){
        ContextDialog::ok(StyleLsp::tr("There is a fatal error between the current"
                                       " editor component and the backend of the syntax server, "
                                       "which may affect the syntax highlighting. \n"
                                       "Please contact the maintainer for troubleshooting "
                                       "to solve the problem!"));
        return;
    }

    int cacheLine = 0;
    for (auto val : tokens) {
        cacheLine += val.start.line;
        qInfo() << "line:" << cacheLine;
        qInfo() << "charStart:" << val.start.character;
        qInfo() << "charLength:" << val.length;
        qInfo() << "tokenType:" << val.tokenType;
        qInfo() << "tokenModifiers:" << val.tokenModifiers;

        auto sciStartPos = StyleLsp::getSciPosition(edit.docPointer(), {cacheLine, val.start.character});
        auto sciEndPos = edit.wordEndPosition(sciStartPos, true);

        auto doc = (Scintilla::Internal::Document*)(edit.docPointer());
        if (sciStartPos != 0 && sciEndPos != doc->Length()) {
            QString sourceText = edit.textRange(sciStartPos, sciEndPos);
            QString tempText = edit.textRange(sciStartPos - 1, sciEndPos + 1);
            // text is word
            if ( ((isCharSymbol(tempText.begin()->toLatin1()) || tempText.startsWith(" "))
                  && (isCharSymbol(tempText.end()->toLatin1()) || tempText.endsWith(" "))) ) {
                qInfo() << "text:" << sourceText;
                edit.indicatorFillRange(sciStartPos, sciEndPos - sciStartPos);
            }
        }
    }
    this->tokensCache = tokens;
}

void StyleLsp::setCompletion(ScintillaEdit &edit, const lsp::CompletionProvider &provider)
{
    if (provider.items.isEmpty())
        return;

    const unsigned char sep = 0x7C; // "|"
    edit.autoCSetSeparator((sptr_t)sep);
    QString inserts;
    for (auto item : provider.items) {
        if (!item.insertText.startsWith(d->editText))
            continue;
        inserts += (item.insertText += sep);
    }
    if (inserts.endsWith(sep)){
        inserts.remove(inserts.count() - 1 , 1);
    }

    edit.autoCShow(d->editCount, inserts.toUtf8());
}

void StyleLsp::setHover(ScintillaEdit &edit, const lsp::Hover &hover)
{
    edit.callTipSetBack(STYLE_DEFAULT);
    if (!hover.contents.value.isEmpty()) {
        edit.callTipShow(d->hoverPos, hover.contents.value.toUtf8().toStdString().c_str());
    }
    d->currEditorCache = nullptr;
}

void StyleLsp::setDefinition(ScintillaEdit &edit, const lsp::DefinitionProvider &provider)
{
    //    auto hoverPos = positionFromPoint(d->definitionPos.x(), d->definitionPos.y());
    //    auto defsStartPos = wordStartPosition(hoverPos, true);
    //    auto defsEndPos = wordEndPosition(hoverPos, true);
    //    auto lspDefsStartPos = StyleLsp::getLspPosition(docPointer(), defsStartPos);
    //    auto lspDefsEndPos = StyleLsp::getLspPosition(docPointer(), defsEndPos);
    //    qInfo() << "defsStartPos.line: " << lspDefsStartPos.line
    //            << "defsStartPos.character: " << lspDefsStartPos.character;
    //    qInfo() << "defsEndPos.line: " << lspDefsEndPos.line
    //            << "defsEndPos.character: " << lspDefsEndPos.character;
    //    //        setIndicatorCurrent(EditTextWidgetStyle::Indic::HoverDefinitionCanJump);
    //    indicatorFillRange(defsStartPos, defsEndPos - defsStartPos);
}


