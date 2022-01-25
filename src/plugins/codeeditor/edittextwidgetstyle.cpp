#include "edittextwidgetstyle.h"
#include "SciLexer.h"
#include "common/common.h"

#include <QDebug>
#include <QHash>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

QString styleSupportFilePath(QString languageID)
{
    return SupportFile::EditorStyle::globalPath(languageID);
}

class EditTextWidgetStylePrivate
{
    friend class EditTextWidgetStyle;
    QHash<int, QString> sciStyleMapping
    {
        { SCE_C_WORD, "Keyword" },
        { SCE_C_WORD2, "Keyword" },
        { SCE_C_NUMBER, "Number" },
        { SCE_C_UUID, "Number" },
        { SCE_C_DEFAULT, "Text" },
        { SCE_C_USERLITERAL, "Text"},
        { SCE_C_TASKMARKER, "Text"},
        { SCE_C_ESCAPESEQUENCE, "Text"},
        { SCE_C_TRIPLEVERBATIM, "Text"},
        { SCE_C_VERBATIM, "Text"},
        { SCE_C_STRINGEOL, "String"},
        { SCE_C_STRING, "String" },
        { SCE_C_CHARACTER, "String" },
        { SCE_C_REGEX, "String"},
        { SCE_C_STRINGRAW, "String"},
        { SCE_C_HASHQUOTEDSTRING, "String"},
        { SCE_C_OPERATOR, "Operators" },
        { SCE_C_IDENTIFIER, "Operators" },
        { SCE_C_PREPROCESSOR, "Preprocessor"},
        { SCE_C_COMMENT, "Comment" },
        { SCE_C_COMMENTLINE, "Comment" },
        { SCE_C_COMMENTLINEDOC, "Comment"},
        { SCE_C_COMMENTDOC, "Comment" },
        { SCE_C_COMMENTDOCKEYWORD, "Comment" },
        { SCE_C_COMMENTDOCKEYWORDERROR, "Comment" },
        { SCE_C_PREPROCESSORCOMMENT, "Comment"},
        { SCE_C_PREPROCESSORCOMMENTDOC, "Comment"},
        { SCE_C_GLOBALCLASS, "Global"}
    };

    QHash<EditTextWidgetStyle::Indic, QString> lspStyleMapping
    {
        { EditTextWidgetStyle::Indic::TokenTypeNamespace, "Namespace" },
        { EditTextWidgetStyle::Indic::TokenTypeType, "Type" },
        { EditTextWidgetStyle::Indic::TokenTypeClass, "Class" },
        { EditTextWidgetStyle::Indic::TokenTypeEnum, "Enumeration" },
        { EditTextWidgetStyle::Indic::TokenTypeInterface, "Function Declaration" },
        { EditTextWidgetStyle::Indic::TokenTypeStruct, "Class"},
        { EditTextWidgetStyle::Indic::TokenTypeTypeParameter, "Text"},
        { EditTextWidgetStyle::Indic::TokenTypeVariable, "Variable"},
        { EditTextWidgetStyle::Indic::TokenTypeProperty, "Text"},
        { EditTextWidgetStyle::Indic::TokenTypeEnumMember, "Text"},
        { EditTextWidgetStyle::Indic::TokenTypeEvent, "Text"},
        { EditTextWidgetStyle::Indic::TokenTypeFunction, "Function" },
        { EditTextWidgetStyle::Indic::TokenTypeMethod, "Function" },
        { EditTextWidgetStyle::Indic::TokenTypeMacro, "Text"},
        { EditTextWidgetStyle::Indic::TokenTypeKeyword, "Keyword"},
        { EditTextWidgetStyle::Indic::TokenTypeModifier, "Text"},
        { EditTextWidgetStyle::Indic::TokenTypeComment, "Comment"},
        { EditTextWidgetStyle::Indic::TokenTypeString, "String" },
        { EditTextWidgetStyle::Indic::TokenTypeNumber, "Number" },
        { EditTextWidgetStyle::Indic::TokenTypeRegexp, "String"},
        { EditTextWidgetStyle::Indic::TokenTypeOperator, "Operators" },
    };

    // "concept" "unknown" not support
    QHash<EditTextWidgetStyle::Indic, QString> lspServerTokenMapping
    {
        { EditTextWidgetStyle::Indic::TokenTypeVariable, "variable"},
        { EditTextWidgetStyle::Indic::TokenTypeParameter, "parameter"},
        { EditTextWidgetStyle::Indic::TokenTypeFunction, "function"},
        { EditTextWidgetStyle::Indic::TokenTypeMethod, "method"},
        { EditTextWidgetStyle::Indic::TokenTypeProperty, "property"},
        { EditTextWidgetStyle::Indic::TokenTypeClass, "class"},
        { EditTextWidgetStyle::Indic::TokenTypeInterface, "interface"},
        { EditTextWidgetStyle::Indic::TokenTypeEnum, "enum"},
        { EditTextWidgetStyle::Indic::TokenTypeEnumMember, "enumMember"},
        { EditTextWidgetStyle::Indic::TokenTypeType, "type"},
        { EditTextWidgetStyle::Indic::TokenTypeNamespace, "namespace"},
        { EditTextWidgetStyle::Indic::TokenTypeTypeParameter, "typeParameter"},
        { EditTextWidgetStyle::Indic::TokenTypeType, "type"},
        { EditTextWidgetStyle::Indic::TokenTypeMacro, "macro"},
        { EditTextWidgetStyle::Indic::TokenTypeComment, "comment"}
    };

    QHash<QString, QJsonObject> globalJsonObjects;

    bool loadStyleFile(const QString &languageID)
    {
        if (!globalJsonObjects.isEmpty())
            return false;

        QString languageStyleFilePath = styleSupportFilePath(languageID);
        QFile file(languageStyleFilePath);

        if (!file.exists()) {
            return false;
        }

        if (file.open(QFile::ReadOnly)){
            QJsonObject obj = QJsonDocument::fromJson(file.readAll()).object();
            globalJsonObjects[languageID] = obj;
            file.close();
        }

        return true;
    }
};

EditTextWidgetStyle::EditTextWidgetStyle()
    : d ( new EditTextWidgetStylePrivate())
{

}

EditTextWidgetStyle::~EditTextWidgetStyle()
{
    if (d)
        delete d;
}

QList<int> EditTextWidgetStyle::sciKeys()
{
    return d->sciStyleMapping.keys();
}

QList<int> EditTextWidgetStyle::scikeys(const QString &name)
{
    return d->sciStyleMapping.keys(name);
}

QList<QString> EditTextWidgetStyle::styleValues()
{
    return d->sciStyleMapping.values();
}

QString EditTextWidgetStyle::styleValue(int key)
{
    return d->sciStyleMapping.value(key);
}

QStringList EditTextWidgetStyle::styleThemes(const QString &languageID)
{
    d->loadStyleFile(languageID);
    return d->globalJsonObjects[languageID].keys();
}

int EditTextWidgetStyle::backgroundSelf(const QString &languageID, const QString &theme)
{
    if (d->globalJsonObjects.isEmpty()) {
        return 0;
    }

    return d->globalJsonObjects[languageID]
            .value(theme).toObject()
            .value("BackgroundSelf").toString().toInt(nullptr, 16);
}

int EditTextWidgetStyle::cursor(const QString &languageID, const QString &theme)
{
    if (d->globalJsonObjects.isEmpty()) {
        return 0;
    }

    return d->globalJsonObjects[languageID]
            .value(theme).toObject()
            .value("Cursor").toString().toInt(nullptr, 16);
}

int EditTextWidgetStyle::fontSize(const QString &languageID,
                                  const QString &theme,
                                  const QString &name)
{
    if (d->globalJsonObjects.isEmpty()) {
        return -1;
    }

    return d->globalJsonObjects[languageID]
            .value(theme).toObject()
            .value(name).toObject()
            .value("FontSize").toInt();
}

int EditTextWidgetStyle::background(const QString &languageID,
                                    const QString &theme,
                                    const QString &name)
{
    if (d->globalJsonObjects.isEmpty()) {
        return 0;
    }

    return d->globalJsonObjects[languageID]
            .value(theme).toObject()
            .value(name).toObject()
            .value("Background").toString().toInt(nullptr, 16);
}

int EditTextWidgetStyle::foreground(const QString &languageID,
                                    const QString &theme,
                                    const QString &name)
{
    if (d->globalJsonObjects.isEmpty()) {
        return 0;
    }

    return d->globalJsonObjects[languageID]
            .value(theme).toObject()
            .value(name).toObject()
            .value("Foreground").toString().toInt(nullptr, 16);
}

EditTextWidgetStyle::Underline EditTextWidgetStyle::underline(const QString &languageID,
                                                              const QString &theme,
                                                              const QString &name)
{
    if (d->globalJsonObjects.isEmpty()) {
        return {};
    }

    auto obj =  d->globalJsonObjects[languageID]
            .value(theme).toObject()
            .value(name).toObject()
            .value("Underline").toObject();
    return { obj.value("Color").toString().toInt(nullptr, 16), obj.value("Style").toInt() };
}

EditTextWidgetStyle::Indic EditTextWidgetStyle::cvtIndic(lsp::SemanticTokenType type)
{
    return (Indic)(type + Indic::TokenTypeNamespace);
}

EditTextWidgetStyle::Indic EditTextWidgetStyle::cvtIndic(lsp::SemanticTokenModifier modifier)
{
    return (Indic)(modifier + Indic::TokenModifierDeclaration);
}

QList<EditTextWidgetStyle::Indic> EditTextWidgetStyle::lspKeys()
{
    return d->lspStyleMapping.keys();
}

EditTextWidgetStyle::Indic EditTextWidgetStyle::tokenKey(const QString &tokenName)
{
    return d->lspServerTokenMapping.key(tokenName);
}

QString EditTextWidgetStyle::styleValue(Indic indic)
{
    return d->lspStyleMapping.value(indic);
}

int EditTextWidgetStyle::fontSize(const QString &languageID,
                                  const QString &theme,
                                  Indic name)
{
    if (d->globalJsonObjects.isEmpty()) {
        return -1;
    }

    return d->globalJsonObjects[languageID]
            .value(theme).toObject()
            .value(d->lspStyleMapping.value(name)).toObject()
            .value("FontSize").toInt();
}

int EditTextWidgetStyle::background(const QString &languageID,
                                    const QString &theme,
                                    Indic name)
{
    if (d->globalJsonObjects.isEmpty()) {
        return 0;
    }

    return d->globalJsonObjects[languageID]
            .value(theme).toObject()
            .value(d->lspStyleMapping.value(name)).toObject()
            .value("Background").toString().toInt(nullptr, 16);
}

int EditTextWidgetStyle::foreground(const QString &languageID,
                                    const QString &theme,
                                    Indic name)
{
    if (d->globalJsonObjects.isEmpty()) {
        return 0;
    }

    return d->globalJsonObjects[languageID]
            .value(theme).toObject()
            .value(d->lspStyleMapping.value(name)).toObject()
            .value("Foreground").toString().toInt(nullptr, 16);
}

EditTextWidgetStyle::Underline EditTextWidgetStyle::underline(const QString &languageID,
                                                              const QString &theme,
                                                              Indic name)
{
    if (d->globalJsonObjects.isEmpty()) {
        return {};
    }

    auto obj =  d->globalJsonObjects[languageID]
            .value(theme).toObject()
            .value(d->lspStyleMapping.value(name)).toObject()
            .value("Underline").toObject();
    return { obj.value("Color").toString().toInt(nullptr, 16), obj.value("Style").toInt() };
}
