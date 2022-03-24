#ifndef STYLELSP_H
#define STYLELSP_H

#include <QObject>
#include <QList>

#include "stylesci.h"

#include "common/common.h"

class ScintillaEditExtern;
class StyleLspPrivate;
class StyleLsp : public QObject
{
    Q_OBJECT
    StyleLspPrivate *const d;
public:

    enum Margin
    {
        LspCustom = StyleSci::Margin::Custom
    };

    enum MarkerNumber
    {
        Error = StyleSci::MarkerNumber::Extern,
        Warning,
        Information,
        Hint,
        ErrorLineBackground,
        WarningLineBackground,
        InformationLineBackground,
        HintLineBackground,
    };

    struct IndicStyleExt
    {
        QMap<int, int> fore;
    };

    enum EOLAnnotation
    {
        RedTextFore = 63,
    };

    static Sci_Position getSciPosition(sptr_t doc, const lsp::Position &pos);
    static lsp::Position getLspPosition(sptr_t doc, sptr_t sciPosition);
    static int getLspCharacter(sptr_t doc, sptr_t sciPosition);
    static bool isCharSymbol(const char ch);

    StyleLsp();
    virtual ~StyleLsp();
    virtual lsp::Client &client();
    virtual void appendEdit(ScintillaEditExtern *editor); //setting main
    virtual ScintillaEditExtern *findSciEdit(const QString &file);
    virtual QString sciEditFile(ScintillaEditExtern * const sciEdit);

    /* Client Info Specialization, matching client portals for various environments.
     * If to use "clangd" client, exist more "clangd" from folder /usr/bin and in other folder,
     * you should make it clear where you should use it.
     * This function does this work, which is similar to the specialized template function
     */
    typedef support_file::Language::ServerInfo ServerInfo;
    virtual ServerInfo clientInfoSpec(ServerInfo info);
    virtual IndicStyleExt symbolIndic(ScintillaEdit &edit,
                                      lsp::SemanticTokenType::type_value token,
                                      QList<lsp::SemanticTokenType::type_index> modifier);
    /*!
     * \brief tokenFromServProvider find local token from lsp protocol init response provider data
     * \param token from token request (tokenFull) key
     * \return local key
     */
    virtual lsp::SemanticTokenType::type_value tokenToDefine(int token);

    virtual void setIndicStyle(ScintillaEdit &edit);
    virtual void setMargin(ScintillaEdit &edit);

    virtual void setDiagnostics(ScintillaEdit &edit, const lsp::DiagnosticsParams &params);
    virtual void cleanDiagnostics(ScintillaEdit &edit);

    virtual void setTokenFull(ScintillaEdit &edit, const QList<lsp::Data> &tokens);
    virtual void cleanTokenFull(ScintillaEdit &edit);

    virtual void setCompletion(ScintillaEdit &edit, const lsp::CompletionProvider &provider);
    virtual void cleanCompletion(ScintillaEdit &edit);

    virtual void setHover(ScintillaEdit &edit, const lsp::Hover &hover);
    virtual void cleanHover(ScintillaEdit &edit);

    virtual void setDefinition(ScintillaEdit &edit, const lsp::DefinitionProvider &provider);
    virtual void cleanDefinition(ScintillaEdit &edit, const Scintilla::Position &pos);

    virtual void doRenameReplace(const lsp::RenameChanges &changes);

private slots:
    void sciTextInserted(Scintilla::Position position,
                         Scintilla::Position length, Scintilla::Position linesAdded,
                         const QByteArray &text, Scintilla::Position line);

    void sciTextDeleted(Scintilla::Position position,
                        Scintilla::Position length, Scintilla::Position linesAdded,
                        const QByteArray &text, Scintilla::Position line);

    void sciHovered(Scintilla::Position position);
    void sciHoverCleaned(Scintilla::Position position);

    void sciDefinitionHover(Scintilla::Position position);
    void sciDefinitionHoverCleaned(Scintilla::Position position);
    void sciIndicClicked(Scintilla::Position position);
    void sciIndicReleased(Scintilla::Position position);
    void sciSelectionMenu(QContextMenuEvent *event);
    void sciReplaced(const QString &file, Scintilla::Position start,
                     Scintilla::Position end, const QString &text);
    void renameRequest(const QString &newText);

private:
    QList<lsp::Data> tokensCache;
    lsp::Client lspClient;
};

#endif // STYLELSP_H
