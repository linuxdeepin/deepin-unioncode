#include "stylelsp.h"
#include "Scintilla.h"
#include "stylesci.h"
#include "stylekeeper.h"
#include "stylecolor.h"
#include "textedittabwidget/textedittabwidget.h"
#include "textedittabwidget/scintillaeditextern.h"
#include "refactorwidget/refactorwidget.h"
#include "renamepopup/renamepopup.h"

#include "services/workspace/workspaceservice.h"
#include "common/common.h"

#include "framework/service/qtclassmanager.h"

#include "Document.h"

#include <QHash>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QMenu>
#include <QLineEdit>
#include <QVBoxLayout>

#include <bitset>

class EditorCache
{
    ScintillaEditExtern *editor = nullptr;
public:
    void clean()
    {
        editor = nullptr;
    }
    bool isEmpty(){
        return editor == nullptr;
    }
    ScintillaEditExtern *getEditor() const
    {
        return editor;
    }
    void setEditor(ScintillaEditExtern *value)
    {
        editor = value;
    }
};

class SciRangeCache
{
    Scintilla::Position start = -1;
    Scintilla::Position end = -1;
public:
    SciRangeCache(Scintilla::Position start, Scintilla::Position end)
        :start(start), end(end){}
    SciRangeCache(){}
    void clean()
    {
        start = -1;
        end = -1;
    }
    bool isEmpty()
    {
        return start != -1 && end != -1;
    }
    Scintilla::Position getStart() const
    {
        return start;
    }
    void setStart(const Scintilla::Position &value)
    {
        start = value;
    }
    Scintilla::Position getEnd() const
    {
        return end;
    }
    void setEnd(const Scintilla::Position &value)
    {
        end = value;
    }
    bool operator == (const SciRangeCache &other)
    {
        return start == other.start
                && end == other.end;
    }
};

class SciPositionCache
{
    Scintilla::Position sciPosition = -1;
public:
    SciPositionCache(){}
    SciPositionCache(const Scintilla::Position &pos)
        : sciPosition(pos){}
    void clean()
    {
        sciPosition = -1;
    }
    bool isEmpty()
    {
        return sciPosition == -1;
    }
    Scintilla::Position getSciPosition() const
    {
        return sciPosition;
    }
    void setSciPosition(const Scintilla::Position &value)
    {
        sciPosition = value;
    }
};

class DefinitionCache : public EditorCache, public SciPositionCache
{
    lsp::DefinitionProvider provider{};
    SciRangeCache textRange{};
    int cursor = 0; //Invalid
public:
    void clean()
    {
        provider.clear();
        cursor = 0;
        EditorCache::clean();
        SciPositionCache::clean();
        textRange.clean();
    }
    bool isEmpty()
    {
        return provider.isEmpty()
                && cursor == 0
                && EditorCache::isEmpty()
                && SciPositionCache::isEmpty()
                && textRange.isEmpty();
    }
    lsp::DefinitionProvider getProvider() const
    {
        return provider;
    }
    void setProvider(const lsp::DefinitionProvider &value)
    {
        provider = value;
    }
    int getCursor() const
    {
        return cursor;
    }
    void setCursor(int value)
    {
        cursor = value;
    }
    SciRangeCache getTextRange() const
    {
        return textRange;
    }
    void setTextRange(const SciRangeCache &value)
    {
        textRange = value;
    }
};

class HoverCache : public EditorCache, public SciPositionCache
{
public:
    void clean()
    {
        EditorCache::clean();
        SciPositionCache::clean();
    }
    bool isEmpty()
    {
        return EditorCache::isEmpty() && SciPositionCache::isEmpty();
    }
};

class RenameCache : public EditorCache
{
    SciPositionCache start;
    SciPositionCache end;
public:
    void clean()
    {
        EditorCache::clean();
        start.clean();
        end.clean();
    }
    bool isEmpty()
    {
        return EditorCache::isEmpty()
                && start.isEmpty()
                && end.isEmpty();
    }
    SciPositionCache getStart() const
    {
        return start;
    }
    void setStart(const SciPositionCache &value)
    {
        start = value;
    }
    SciPositionCache getEnd() const
    {
        return end;
    }
    void setEnd(const SciPositionCache &value)
    {
        end = value;
    }
};

class CompletionCache : public EditorCache {};

class StyleLspPrivate
{
    QHash<QString, ScintillaEditExtern*> editors;
    // response not return url, cache send to callback method
    CompletionCache completionCache;
    DefinitionCache definitionCache;
    HoverCache hoverCache;
    RenamePopup renamePopup;
    RenameCache renameCache;
    EditorCache editCache;
    lsp::SemanticTokensProvider tokensProvider;
    QString editText = "";
    uint editCount = 0;
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
    QObject::connect(&d->renamePopup, &RenamePopup::editingFinished,
                     this, &StyleLsp::renameRequest, Qt::UniqueConnection);

    QObject::connect(&client(), QOverload<const lsp::RenameChanges&>::of(&lsp::Client::requestResult),
                     this, &StyleLsp::doRenameReplace);

    QObject::connect(&client(), QOverload<const lsp::References&>::of(&lsp::Client::requestResult),
                     RefactorWidget::instance(), &RefactorWidget::displayReference);
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

void StyleLsp::sciTextInserted(Scintilla::Position position,
                               Scintilla::Position length, Scintilla::Position linesAdded,
                               const QByteArray &text, Scintilla::Position line)
{
    auto edit = qobject_cast<ScintillaEditExtern*>(sender());
    if (!edit)
        return;

    edit->indicatorClearRange(0, edit->length()); // clean all indicator range style

    cleanCompletion(*edit);
    cleanDiagnostics(*edit);
    client().changeRequest(edit->file(), edit->textRange(0, edit->length()));

    d->editCache.setEditor(edit);
    client().docSemanticTokensFull(edit->file());

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

    edit->indicatorClearRange(0, edit->length()); // clean all indicator range style

    qInfo() << "position" << position
            << "length" << length
            << "linesAdded" << linesAdded
            << "text" << text
            << "line" << line;

    cleanCompletion(*edit);
    cleanDiagnostics(*edit);
    client().changeRequest(edit->file(), edit->textRange(0, edit->length()));

    d->editCache.setEditor(edit);
    client().docSemanticTokensFull(edit->file());

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

    d->hoverCache.setEditor(edit);
    d->hoverCache.setSciPosition(position);

    auto lspPostion = getLspPosition(edit->docPointer(), d->hoverCache.getSciPosition());
    client().docHoverRequest(edit->file(), lspPostion);
}

void StyleLsp::sciHoverCleaned(Scintilla::Position position)
{
    Q_UNUSED(position);
    auto edit = qobject_cast<ScintillaEditExtern*>(sender());
    if (!edit)
        return;

    cleanHover(*edit);
    d->hoverCache.clean();
}

void StyleLsp::sciDefinitionHover(Scintilla::Position position)
{
    auto edit = qobject_cast<ScintillaEditExtern*>(sender());
    if (!edit)
        return;

    if (edit->isLeave())
        return;

    // 判断缓存文字范围
    auto afterTextRange = d->definitionCache.getTextRange();
    auto currTextRange = SciRangeCache{edit->wordStartPosition(position, true), edit->wordEndPosition(position, true)};
    auto isSameTextRange = afterTextRange == currTextRange;

    // 编辑器不相等, 直接刷新数据
    if  (edit != d->definitionCache.getEditor()) {
        d->definitionCache.setEditor(edit);
        d->definitionCache.setSciPosition(position);
        d->definitionCache.setTextRange(currTextRange);
        d->definitionCache.setProvider({}); // 清空Provider
        edit->setCursor(-1); // 恢复鼠标状态
        d->definitionCache.setCursor(edit->cursor());
    } else { // 编辑器相等
        if (isSameTextRange) { // 相同的关键字不再触发Definition的绘制
            d->definitionCache.setSciPosition(position); // 更新坐标点
            return;
        } else {
            d->definitionCache.setTextRange(currTextRange);
            d->definitionCache.setSciPosition(position);
            d->definitionCache.setProvider({}); // 清空Provider
        }
    }
    auto lspPostion = getLspPosition(edit->docPointer(), d->definitionCache.getSciPosition());
    client().definitionRequest(edit->file(), lspPostion);
}

void StyleLsp::sciDefinitionHoverCleaned(Scintilla::Position position)
{
    Q_UNUSED(position);
    auto edit = qobject_cast<ScintillaEditExtern*>(sender());
    if (!edit)
        return;

    if (edit != d->definitionCache.getEditor()){
        return;
    }

    // 判断缓存文字范围
    auto afterTextRange = d->definitionCache.getTextRange();
    auto currTextRange = SciRangeCache{edit->wordStartPosition(position, true), edit->wordEndPosition(position, true)};
    auto isSameTextRange = afterTextRange == currTextRange;
    if (!d->definitionCache.isEmpty() && !isSameTextRange) {
        if (d->definitionCache.getEditor()) {
            cleanDefinition(*d->definitionCache.getEditor(), d->definitionCache.getSciPosition());
        }
        d->definitionCache.clean();
    }
}

void StyleLsp::sciIndicClicked(Scintilla::Position position)
{
    Q_UNUSED(position);
    auto edit = qobject_cast<ScintillaEditExtern*>(sender());
    if (!edit)
        return;

    std::bitset<32> flags(edit->indicatorAllOnFor(position));
    if (flags[INDIC_COMPOSITIONTHICK]) {
        if (d->definitionCache.getProvider().count() > 0) {
            auto providerAtOne = d->definitionCache.getProvider().first();
            TextEditTabWidget::instance()->jumpToLine(providerAtOne.fileUrl.toLocalFile(), providerAtOne.range.end.line);
            cleanDefinition(*edit, position);
        }
    }
}

void StyleLsp::sciIndicReleased(Scintilla::Position position)
{
    Q_UNUSED(position);
}

void StyleLsp::sciSelectionMenu(QContextMenuEvent *event)
{
    auto edit = qobject_cast<ScintillaEditExtern*>(sender());
    if (!edit)
        return;

    d->renameCache.setEditor(edit);
    d->renameCache.setStart(edit->selectionStart());
    d->renameCache.setEnd(edit->selectionEnd());

    QPoint showPos = edit->mapToGlobal(event->pos());
    QByteArray sourceText = edit->textRange(
                edit->wordStartPosition(edit->selectionStart(), true),
                edit->wordEndPosition(edit->selectionEnd(), true));

    QMenu contextMenu;
    QMenu refactor(QMenu::tr("Refactor"));

    QAction *renameAction = refactor.addAction(QAction::tr("Rename"));
    QObject::connect(renameAction, &QAction::triggered, [&](){
        d->renamePopup.setOldName(sourceText);
        d->renamePopup.exec(showPos);
    });
    contextMenu.addMenu(&refactor);

    QAction *findSymbol = contextMenu.addAction(QAction::tr("Find Usages"));
    QObject::connect(findSymbol, &QAction::triggered, [&](){
        this->client().referencesRequest(edit->file(), getLspPosition(edit->docPointer(), edit->selectionStart()));
    });

    contextMenu.move(showPos);
    contextMenu.exec();
}

void StyleLsp::sciReplaced(const QString &file, Scintilla::Position start, Scintilla::Position end, const QString &text)
{
    auto edit = qobject_cast<ScintillaEditExtern*>(sender());
    if (!edit)
        return;

    Q_UNUSED(start);
    Q_UNUSED(end);
    client().changeRequest(file, edit->textRange(0, edit->length()));
}

void StyleLsp::renameRequest(const QString &newText)
{
    auto editor = d->renameCache.getEditor();
    auto sciPostion = d->renameCache.getStart().getSciPosition();
    if (editor) {
        client().renameRequest(editor->file(),
                               getLspPosition(editor->docPointer(), sciPostion),
                               newText);
    }
}

lsp::Client &StyleLsp::client()
{
    if (lspClient.program().isEmpty()
            && lspClient.state() == QProcess::ProcessState::NotRunning) {
        support_file::Language::initialize();
        auto serverInfo = support_file::Language::sever(StyleKeeper::key(this));

        serverInfo = clientInfoSpec(serverInfo);

        if (!serverInfo.progrma.isEmpty()) {
            lspClient.setProgram(serverInfo.progrma);
            lspClient.setArguments(serverInfo.arguments);
            lspClient.start();
        }
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
    QObject::connect(editor, &ScintillaEditExtern::definitionHover, this, &StyleLsp::sciDefinitionHover);
    QObject::connect(editor, &ScintillaEditExtern::definitionHoverCleaned, this, &StyleLsp::sciDefinitionHoverCleaned);
    QObject::connect(editor, &ScintillaEditExtern::indicClicked, this, &StyleLsp::sciIndicClicked);
    QObject::connect(editor, &ScintillaEditExtern::indicReleased, this, &StyleLsp::sciIndicReleased);
    QObject::connect(editor, &ScintillaEditExtern::selectionMenu, this, &StyleLsp::sciSelectionMenu);
    QObject::connect(editor, &ScintillaEditExtern::replaceed, this, &StyleLsp::sciReplaced);

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
        if (!editor) { return; }
        this->cleanDiagnostics(*editor);
        this->setDiagnostics(*editor, params);
    });

    QObject::connect(&client(), QOverload<const QList<lsp::Data>&>::of(&lsp::Client::requestResult),
                     this, [=](const QList<lsp::Data> &data)
    {
        this->setTokenFull(*d->editCache.getEditor(), data);
    });

    QObject::connect(&client(), QOverload<const lsp::SemanticTokensProvider&>::of(&lsp::Client::requestResult),
                     this, [=](const lsp::SemanticTokensProvider& provider){
        d->tokensProvider = provider;
    });

    QObject::connect(&client(), QOverload<const lsp::Hover&>::of(&lsp::Client::requestResult),
                     this, [=](const lsp::Hover& hover){
        if (!d->hoverCache.getEditor()) { return; }
        setHover(*d->hoverCache.getEditor(), hover);
    });

    QObject::connect(&client(), QOverload<const lsp::CompletionProvider&>::of(&lsp::Client::requestResult),
                     this, [=](const lsp::CompletionProvider& provider){
        if (!d->completionCache.getEditor()) { return; }
        setCompletion(*d->completionCache.getEditor(), provider);
    });

    QObject::connect(&client(), QOverload<const lsp::DefinitionProvider&>::of(&lsp::Client::requestResult),
                     this, [=](const lsp::DefinitionProvider& provider){
        if (!d->definitionCache.getEditor()) { return; }
        setDefinition(*d->definitionCache.getEditor(), provider);
    });

    QObject::connect(&client(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [=](int code, auto status) {
        qInfo() << code << status;
    });

    QObject::connect(RefactorWidget::instance(), &RefactorWidget::doubleClicked,
                     this, [=](const QString &filePath, const lsp::Range &range){
        TextEditTabWidget::instance()->jumpToRange(filePath, range);
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

    d->editCache.setEditor(editor); // 事件回调缓存
    client().docSemanticTokensFull(editor->file());
}

QString StyleLsp::sciEditFile(ScintillaEditExtern * const sciEdit)
{
    return d->editors.key(sciEdit);
}

StyleLsp::ServerInfo StyleLsp::clientInfoSpec(StyleLsp::ServerInfo info)
{
    return info;
}

StyleLsp::IndicStyleExt StyleLsp::symbolIndic(ScintillaEdit &edit,
                                              lsp::SemanticTokenType::type_value token,
                                              QList<lsp::SemanticTokenType::type_index> modifier)
{
    Q_UNUSED(edit);
    Q_UNUSED(token);
    Q_UNUSED(modifier);
    return {};
}

lsp::SemanticTokenType::type_value StyleLsp::tokenToDefine(int token)
{
    if (0 <= token && token < d->tokensProvider.legend.tokenTypes.size())
        return d->tokensProvider.legend.tokenTypes[token];
    return {};
}

void StyleLsp::setIndicStyle(ScintillaEdit &edit)
{
    edit.indicSetStyle(INDIC_PLAIN, INDIC_PLAIN);
    edit.indicSetStyle(INDIC_SQUIGGLE, INDIC_SQUIGGLE);
    edit.indicSetStyle(INDIC_TT, INDIC_TT);
    edit.indicSetStyle(INDIC_DIAGONAL, INDIC_DIAGONAL);
    edit.indicSetStyle(INDIC_STRIKE, INDIC_STRIKE);
    edit.indicSetStyle(INDIC_HIDDEN, INDIC_HIDDEN);
    edit.indicSetStyle(INDIC_BOX, INDIC_BOX);
    edit.indicSetStyle(INDIC_ROUNDBOX, INDIC_ROUNDBOX);
    edit.indicSetStyle(INDIC_STRAIGHTBOX, INDIC_STRAIGHTBOX);
    edit.indicSetStyle(INDIC_FULLBOX, INDIC_FULLBOX);
    edit.indicSetStyle(INDIC_DASH, INDIC_DASH);
    edit.indicSetStyle(INDIC_DOTS, INDIC_DOTS);
    edit.indicSetStyle(INDIC_SQUIGGLELOW, INDIC_SQUIGGLELOW);
    edit.indicSetStyle(INDIC_DOTBOX, INDIC_DOTBOX);
    edit.indicSetStyle(INDIC_GRADIENT, INDIC_GRADIENT);
    edit.indicSetStyle(INDIC_GRADIENTCENTRE, INDIC_GRADIENTCENTRE);
    edit.indicSetStyle(INDIC_SQUIGGLEPIXMAP, INDIC_SQUIGGLEPIXMAP);
    edit.indicSetStyle(INDIC_COMPOSITIONTHICK, INDIC_COMPOSITIONTHICK);
    edit.indicSetStyle(INDIC_COMPOSITIONTHIN, INDIC_COMPOSITIONTHIN);
    edit.indicSetStyle(INDIC_TEXTFORE, INDIC_TEXTFORE);
    edit.indicSetStyle(INDIC_POINT, INDIC_POINT);
    edit.indicSetStyle(INDIC_POINTCHARACTER, INDIC_POINTCHARACTER);
}

void StyleLsp::setMargin(ScintillaEdit &edit)
{
    edit.setMargins(SC_MAX_MARGIN);
    edit.setMarginTypeN(Margin::LspCustom, SC_MARGIN_SYMBOL);
    edit.setMarginWidthN(Margin::LspCustom, 16);
    edit.setMarginMaskN(Margin::LspCustom, 1 << MarkerNumber::Error | 1 << MarkerNumber::ErrorLineBackground
                        | 1 << MarkerNumber::Warning | 1 << MarkerNumber::WarningLineBackground
                        | 1 << MarkerNumber::Information | 1 << MarkerNumber::InformationLineBackground
                        | 1 << MarkerNumber::Hint | 1 << MarkerNumber::HintLineBackground);

    edit.markerDefine(MarkerNumber::Error, SC_MARK_CIRCLE);
    edit.markerDefine(MarkerNumber::Warning, SC_MARK_CIRCLE);
    edit.markerDefine(MarkerNumber::Information, SC_MARK_CIRCLE);
    edit.markerDefine(MarkerNumber::Hint, SC_MARK_CIRCLE);

    edit.markerDefine(MarkerNumber::ErrorLineBackground, SC_MARK_BACKGROUND);
    edit.markerDefine(MarkerNumber::WarningLineBackground, SC_MARK_BACKGROUND);
    edit.markerDefine(MarkerNumber::InformationLineBackground, SC_MARK_BACKGROUND);
    edit.markerDefine(MarkerNumber::HintLineBackground, SC_MARK_BACKGROUND);

    edit.markerSetFore(MarkerNumber::Error, StyleColor::color(StyleColor::Table::get()->Red));
    edit.markerSetBackTranslucent(MarkerNumber::Error, 0);
    edit.markerSetStrokeWidth(MarkerNumber::Error, 300);

    edit.markerSetFore(MarkerNumber::Warning, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetBackTranslucent(MarkerNumber::Warning, 0);
    edit.markerSetStrokeWidth(MarkerNumber::Warning, 300);

    edit.markerSetFore(MarkerNumber::Information, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetBackTranslucent(MarkerNumber::Information, 0);
    edit.markerSetStrokeWidth(MarkerNumber::Information, 300);

    edit.markerSetFore(MarkerNumber::Hint, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetBackTranslucent(MarkerNumber::Hint, 0);
    edit.markerSetStrokeWidth(MarkerNumber::Hint, 300);

    edit.markerSetFore(MarkerNumber::ErrorLineBackground, StyleColor::color(StyleColor::Table::get()->Red));
    edit.markerSetBack(MarkerNumber::ErrorLineBackground, StyleColor::color(StyleColor::Table::get()->Red));
    edit.markerSetAlpha(MarkerNumber::ErrorLineBackground, 0x22);
    edit.markerSetFore(MarkerNumber::WarningLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetBack(MarkerNumber::WarningLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetAlpha(MarkerNumber::WarningLineBackground, 0x22);
    edit.markerSetFore(MarkerNumber::InformationLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetBack(MarkerNumber::InformationLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetAlpha(MarkerNumber::InformationLineBackground, 0x22);
    edit.markerSetFore(MarkerNumber::HintLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetBack(MarkerNumber::HintLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    edit.markerSetAlpha(MarkerNumber::HintLineBackground, 0x22);
}

void StyleLsp::setDiagnostics(ScintillaEdit &edit, const lsp::DiagnosticsParams &params)
{
    for (auto val : params.diagnostics) {
        if (val.severity == lsp::Diagnostic::Severity::Error) { // error
            Sci_Position startPos = getSciPosition(edit.docPointer(), val.range.start);
            Sci_Position endPos = getSciPosition(edit.docPointer(), val.range.end);
            edit.setIndicatorCurrent(INDIC_SQUIGGLE);
            edit.indicSetFore(INDIC_SQUIGGLE, StyleColor::color(StyleColor::Table::get()->Red));
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

void StyleLsp::cleanDiagnostics(ScintillaEdit &edit)
{
    edit.eOLAnnotationClearAll();
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
        if (sciStartPos != 0 && sciEndPos != edit.length()) {
            QString sourceText = edit.textRange(sciStartPos, sciEndPos);
            int wordLength = sciEndPos - sciStartPos;
            qInfo() << "text:" << sourceText;
            // text is word
            if (!sourceText.isEmpty() && wordLength == val.length) {
                QString tokenValue = tokenToDefine(val.tokenType);
                qInfo() << "tokenValue:" << tokenValue;
                auto indics = symbolIndic(edit, tokenValue, val.tokenModifiers);
                for (int i = 0; i < INDIC_MAX; i++) {
                    if (indics.fore.keys().contains(i)) {
                        qInfo() << "fillRangeColor:" << hex << indics.fore[i];
                        edit.setIndicatorCurrent(i);
                        edit.indicSetFlags(i, SC_INDICFLAG_VALUEFORE);
                        edit.setIndicatorValue(indics.fore[i]);
                        edit.indicatorFillRange(sciStartPos, wordLength);
                    }
                }
            }
        }
    }
    this->tokensCache = tokens;
}

void StyleLsp::cleanTokenFull(ScintillaEdit &edit)
{
    Q_UNUSED(edit);
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

void StyleLsp::cleanCompletion(ScintillaEdit &edit)
{

}

void StyleLsp::setHover(ScintillaEdit &edit, const lsp::Hover &hover)
{
    edit.callTipSetBack(STYLE_DEFAULT);
    if (!hover.contents.value.isEmpty()) {
        edit.callTipShow(d->hoverCache.getSciPosition(), hover.contents.value.toUtf8().toStdString().c_str());
    };
    d->hoverCache.clean();
}

void StyleLsp::cleanHover(ScintillaEdit &edit)
{
    edit.callTipCancel();
}

void StyleLsp::setDefinition(ScintillaEdit &edit, const lsp::DefinitionProvider &provider)
{
    d->definitionCache.setProvider(provider);
    auto sciStartPos = edit.wordStartPosition(d->definitionCache.getSciPosition(), true);
    auto sciEndPos = edit.wordEndPosition(d->definitionCache.getSciPosition(), true);

    if (provider.count() >= 1) {
        edit.setIndicatorCurrent(INDIC_COMPOSITIONTHICK);
        edit.indicSetFore(INDIC_COMPOSITIONTHICK, edit.styleFore(0));
        edit.indicatorFillRange(sciStartPos, sciEndPos - sciStartPos);
        if (edit.cursor() != 8) {
            d->definitionCache.setCursor(edit.cursor());
            edit.setCursor(8); // hand from Scintilla platfrom.h
        }
    }
}

void StyleLsp::cleanDefinition(ScintillaEdit &edit, const Scintilla::Position &pos)
{
    std::bitset<32> flags(edit.indicatorAllOnFor(pos));
    if (flags[INDIC_COMPOSITIONTHICK]) {
        // auto hotSpotStart = edit.indicatorStart(INDIC_COMPOSITIONTHICK, pos);
        // auto hotSpotEnd = edit.indicatorEnd(INDIC_COMPOSITIONTHICK, pos);
        edit.setCursor(d->definitionCache.getCursor());
        // edit.indicatorClearRange(hotSpotStart, hotSpotEnd);
        edit.indicatorClearRange(0, edit.length());
    }
}

void StyleLsp::doRenameReplace(const lsp::RenameChanges &changes)
{
    for (auto change : changes) {
        QString filePath = change.documentUri.toLocalFile();
        for (auto edit : change.edits) {
            TextEditTabWidget::instance()->replaceRange(filePath, edit.range, edit.newText);
        }
    }
}
