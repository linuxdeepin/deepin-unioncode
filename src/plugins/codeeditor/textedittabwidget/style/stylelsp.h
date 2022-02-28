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

    enum DiagnosticsStyle
    {
        RedSquiggle = 0,
    };

    enum EOLAnnotation
    {
        RedTextFore = 63,
    };

    enum HotSpot{
        CanJumpLink = 0
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

    virtual void setIndicStyle(ScintillaEdit &edit);
    virtual void setMargin(ScintillaEdit &edit);

    virtual void setDiagnostics(ScintillaEdit &edit, const lsp::DiagnosticsParams &params);
    virtual void setTokenFull(ScintillaEdit &edit, const QList<lsp::Data> &tokens);
    virtual void setCompletion(ScintillaEdit &edit, const lsp::CompletionProvider &provider);
    virtual void setHover(ScintillaEdit &edit, const lsp::Hover &hover);
    virtual void setDefinition(ScintillaEdit &edit, const lsp::DefinitionProvider &provider);

    virtual void cleanDiagnostics(ScintillaEdit &edit);
    virtual void cleanTokenFull(ScintillaEdit &edit);
    virtual void cleanCompletion(ScintillaEdit &edit);
    virtual void cleanHover(ScintillaEdit &edit);
    virtual void cleanDefinition(ScintillaEdit &edit);

private slots:
    void sciTextInserted(Scintilla::Position position,
                         Scintilla::Position length, Scintilla::Position linesAdded,
                         const QByteArray &text, Scintilla::Position line);

    void sciTextDeleted(Scintilla::Position position,
                        Scintilla::Position length, Scintilla::Position linesAdded,
                        const QByteArray &text, Scintilla::Position line);

    void sciHovered(Scintilla::Position position);
    void sciHoverCleaned(Scintilla::Position position);

private:
    QList<lsp::Data> tokensCache;
    lsp::Client lspClient;
};

#endif // STYLELSP_H
