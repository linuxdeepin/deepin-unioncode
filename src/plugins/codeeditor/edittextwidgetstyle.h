#ifndef EDITTEXTWIDGETSTYLE_H
#define EDITTEXTWIDGETSTYLE_H

#include <QObject>

#include "common/common.h"

#include "Scintilla.h"

class EditTextWidgetStylePrivate;
class EditTextWidgetStyle final : public QObject
{
    EditTextWidgetStylePrivate* const d;
public:
    enum Indic
    {
        DiagnosticUnkown = 0,
        DiagnosticError = 1,
        DiagnosticWarning = 2,
        DiagnosticInfo = 3,
        DiagnosticHint = 4,
        TokenTypeNamespace = 5,
        TokenTypeType,
        TokenTypeClass,
        TokenTypeEnum,
        TokenTypeInterface,
        TokenTypeStruct,
        TokenTypeTypeParameter,
        TokenTypeParameter,
        TokenTypeVariable,
        TokenTypeProperty,
        TokenTypeEnumMember,
        TokenTypeEvent,
        TokenTypeFunction,
        TokenTypeMethod,
        TokenTypeMacro,
        TokenTypeKeyword,
        TokenTypeModifier,
        TokenTypeComment,
        TokenTypeString,
        TokenTypeNumber,
        TokenTypeRegexp,
        TokenTypeOperator,
        TokenModifierDeclaration,
        TokenModifierDefinition,
        TokenModifierReadonly,
        TokenModifierStatic,
        TokenModifierDeprecated,
        TokenModifierAbstract,
        TokenModifierAsync,
        TokenModifierModification,
        TokenModifierDocumentation,
        TokenModifierDefaultLibrary
    };

    struct Underline
    {
        int colorHex;
        int style;
    };

    EditTextWidgetStyle();
    ~EditTextWidgetStyle();

    //find sci definitions mapping
    virtual int backgroundSelf(const QString &languageID, const QString &theme);
    virtual int cursor(const QString &languageID, const QString &theme);

    //scintiall style interface
    virtual QList<int> sciKeys();
    virtual QList<int> scikeys(const QString &name);
    virtual QList<QString> styleValues();
    virtual QString styleValue(int key);
    virtual QStringList styleThemes(const QString &languageID);
    virtual int fontSize(const QString &languageID, const QString &theme, const QString &name);
    virtual int background(const QString &languageID, const QString &theme, const QString &name);
    virtual int foreground(const QString &languageID, const QString &theme, const QString &name);
    virtual Underline underline(const QString &languageID, const QString &theme, const QString &name);

    //lsp interface
    Indic cvtIndic(lsp::SemanticTokenType type);
    Indic cvtIndic(lsp::SemanticTokenModifier modifier);
    virtual QList<Indic> lspKeys();
    virtual Indic tokenKey(const QString &tokenName);
    virtual QString styleValue(Indic indic);
    virtual int fontSize(const QString &languageID, const QString &theme, Indic name);
    virtual int background(const QString &languageID, const QString &theme, Indic name);
    virtual int foreground(const QString &languageID, const QString &theme, Indic name);
    virtual Underline underline(const QString &languageID, const QString &theme, Indic name);
};

#endif // EDITTEXTWIDGETSTYLE_H
