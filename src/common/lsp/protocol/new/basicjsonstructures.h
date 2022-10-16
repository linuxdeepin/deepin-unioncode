/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef BASICJSONSTRUCTURES_H
#define BASICJSONSTRUCTURES_H

#include "common/type/menuext.h"

#include <any>
#include <map>
#include <optional>

// std::optional has default set empty is ( 'value' | null )

namespace newlsp {

extern const QString K_ID;
extern const QString K_JSON_RPC;
extern const QString V_2_0;
extern const QString K_METHOD;
extern const QString K_RESULT;
extern const QString K_PARAMS;
extern const QString H_CONTENT_LENGTH;
extern const QString H_CONTENT_TYPE;
extern const QString H_CHARSET;
extern const QString RK_CONTENT_LENGTH; // RegExp Key
extern const QString RK_CONTENT_TYPE;
extern const QString RK_CHARSET;

QString methodData(int id, const QString &method, const QJsonObject &params);
QString notificationData(const QString &method, const QJsonObject &params);

template<class T>
static bool any_contrast(const std::any &any)
{
    if (any.type() == std::any(T()).type()) {
        return true;
    }
    return false;
}

std::string toJsonValueStr(unsigned int value);
std::string toJsonValueStr(int value);
std::string toJsonValueStr(bool value);
std::string toJsonValueStr(float value);
std::string toJsonValueStr(double value);
std::string toJsonValueStr(const std::string &value);
std::string toJsonValueStr(const std::vector<int> &vecInt);
std::string toJsonValueStr(const std::vector<std::string> &vecString);

namespace json
{
std::string addScope(const std::string &src);
std::string delScope(const std::string &obj);
std::string mergeObjs(const std::vector<std::string> &objs);
std::string formatKey(const std::string &key);

template<class T>
struct KV
{
    std::string key;
    T value;
    typedef std::string type_first;
    typedef T type_second;
    KV() = delete;
    KV(const std::string &key, const T &value)
        : key(key), value(value){}
};

template <class T>
std::string addValue(const std::string &src, const KV<T> &kv)
{
    std::string temp;
    if (kv.key.empty())
        return temp;

    temp = formatKey(kv.key) + ":"  + toJsonValueStr(kv.value);

    if (!src.empty())
        return src + "," + temp;
    else
        return temp;
}

template <class T>
std::string addValue(const std::string &src, const KV<std::optional<T>> &kv)
{
    std::string ret;
    if (kv.value) {
        ret = addValue(src, json::KV{kv.key, kv.value.value()});
    }
    return ret;
}

template <class T>
std::string addValue(const std::string &src, const KV<std::vector<T>> &kv)
{
    std::string temp;
    if (kv.key.empty())
        return temp;

    if (kv.value.size() < 0)
        return temp;

    temp += "[";
    for (int i = 0; i < kv.value.size(); i++) {
        temp += toJsonValueStr(kv.value[i]);
        if (i < kv.value.size() - 1)
            temp += ",";
    }
    temp += "]";
    temp = formatKey(kv.key) + ":" + temp;
    if (!src.empty())
        return src + "," + temp;
    else
        return temp;
}

template <class T>
std::string addValue(const std::string &src, const KV<std::optional<std::vector<T>>> &kv)
{
    std::string ret;
    if (kv.value) {
        ret = addValue(src, json::KV{kv.key, kv.value.value()});
    }
    return ret;
}

};

namespace Enum {

enum_def(ResourceOperationKind, std::string)
{
    enum_exp Create = "create";
    enum_exp Rename = "rename";
    enum_exp Delete = "delete";
};

enum_def(FailureHandlingKind, std::string)
{
    enum_exp Abort = "abort";
    enum_exp Transactional = "transactional";
    enum_exp Undo = "undo";
    enum_exp TextOnlyTransactional = "textOnlyTransactional";
};

enum_def(SymbolKind, int)
{
    enum_exp File = 1;
    enum_exp Module = 2;
    enum_exp Namespace = 3;
    enum_exp Package = 4;
    enum_exp Class = 5;
    enum_exp Method = 6;
    enum_exp Property = 7;
    enum_exp Field = 8;
    enum_exp Constructor = 9;
    enum_exp Enum = 10;
    enum_exp Interface = 11;
    enum_exp Function = 12;
    enum_exp Variable = 13;
    enum_exp Constant = 14;
    enum_exp String = 15;
    enum_exp Number = 16;
    enum_exp Boolean = 17;
    enum_exp Array = 18;
    enum_exp Object = 19;
    enum_exp Key = 20;
    enum_exp Null = 21;
    enum_exp EnumMember = 22;
    enum_exp Struct = 23;
    enum_exp Event = 24;
    enum_exp Operator = 25;
    enum_exp TypeParameter = 26;
};

enum_def(SymbolTag, int)
{
    enum_exp Deprecated = 1;
};

enum_def(MarkupKind, std::string)
{
    enum_exp Markdown = "markdown";
    enum_exp PlainText = "plaintext";
};

enum_def(CompletionItemTag, int)
{
    enum_exp Deprecated = 1;
};

enum_def(InsertTextMode, int)
{
    enum_exp asIs = 1;
    enum_exp adjustIndentation = 2;
};

enum_def(CompletionItemKind, int)
{
    enum_exp Text = 1;
    enum_exp Method = 2;
    enum_exp Function = 3;
    enum_exp Constructor = 4;
    enum_exp Field = 5;
    enum_exp Variable = 6;
    enum_exp Class = 7;
    enum_exp Interface = 8;
    enum_exp Module = 9;
    enum_exp Property = 10;
    enum_exp Unit = 11;
    enum_exp Value = 12;
    enum_exp Enum = 13;
    enum_exp Keyword = 14;
    enum_exp Snippet = 15;
    enum_exp Color = 16;
    enum_exp File = 17;
    enum_exp Reference = 18;
    enum_exp Folder = 19;
    enum_exp EnumMember = 20;
    enum_exp Constant = 21;
    enum_exp Struct = 22;
    enum_exp Event = 23;
    enum_exp Operator = 24;
    enum_exp TypeParameter = 25;
};

enum_def(CodeActionKind, std::string)
{
    enum_exp Empty = "";
    enum_exp QuickFix = "quickfix";
    enum_exp Refactor = "refactor";
    enum_exp RefactorExtract = "refactor.extract";
    enum_exp RefactorInline = "refactor.inline";
    enum_exp RefactorRewrite = "refactor.rewrite";
    enum_exp Source = "source";
    enum_exp SourceOrganizeImports = "source.organizeImports";
    enum_exp SourceFixAll = "source.fixAll";
};

enum_def(PrepareSupportDefaultBehavior, int)
{
    enum_exp Identifier = 1;
};

enum_def(DiagnosticTag, int)
{
    enum_exp Unnecessary = 1;
    enum_exp Deprecated = 2;
};

enum_def(FoldingRangeKind, std::string)
{
    enum_exp Comment = "comment";
    enum_exp Imports = "imports";
    enum_exp Region = "region";
};

enum_def(TokenFormat, std::string)
{
    enum_exp Relative = "relative";
};

enum_def(PositionEncodingKind, std::string)
{
    enum_exp UTF8 = "utf-8";
    enum_exp UTF16 = "utf-16";
    enum_exp UTF32 = "utf-32";
};

enum_def(TraceValue, std::string)
{
    enum_exp Off = "off";
    enum_exp Message = "messages";
    enum_exp Verbose = "verbose";
};

enum_def(SemanticTokenTypes, std::string)
{
    enum_exp Namespace = "namespace";
    /**
     * Represents a generic type. Acts as a fallback for types which
     * can"t be mapped to a specific type like class or enum.
     */
    enum_exp Type = "type";
    enum_exp Class = "class";
    enum_exp Enum = "enum";
    enum_exp Interface = "interface";
    enum_exp Struct = "struct";
    enum_exp TypeParameter = "typeParameter";
    enum_exp Parameter = "parameter";
    enum_exp Variable = "variable";
    enum_exp Property = "property";
    enum_exp EnumMember = "enumMember";
    enum_exp Event = "event";
    enum_exp Function = "function";
    enum_exp Method = "method";
    enum_exp Macro = "macro";
    enum_exp Keyword = "keyword";
    enum_exp Modifier = "modifier";
    enum_exp Comment = "comment";
    enum_exp String = "string";
    enum_exp Number = "number";
    enum_exp Regexp = "regexp";
    enum_exp Operator = "operator";
    /**
     * @since 3.17.0
     */
    enum_exp Decorator = "decorator";
};


enum_def(SemanticTokenModifiers, std::string)
{
    enum_exp Declaration = "declaration";
    enum_exp Definition = "definition";
    enum_exp Readonly = "readonly";
    enum_exp Static = "static";
    enum_exp Deprecated = "deprecated";
    enum_exp Abstract = "abstract";
    enum_exp Async = "async";
    enum_exp Modification = "modification";
    enum_exp Documentation = "documentation";
    enum_exp DefaultLibrary = "defaultLibrary";
};

enum_def(Properties, std::string)
{
    enum_exp label_location = "label.location";
    enum_exp edit = "edit";
};

enum_def(DiagnosticSeverity, int)
{
    enum_exp Error = 1;
    enum_exp Warning = 2;
    enum_exp Information = 3;
    enum_exp Hint = 4;
};

enum_def(CompletionTriggerKind, int) {
    enum_exp Invoked = 1;
    enum_exp TriggerCharacter = 2;
    enum_exp TriggerForIncompleteCompletions = 3;
};

enum_def(InsertTextFormat, int) {
    enum_exp PlainText = 1;
    enum_exp Snippet = 2;
};

enum_def(InlayHintKind, int)
{
    enum_exp Type = 1;
    enum_exp Parameter = 2;
};

enum_def(MonikerKind, std::string){
    enum_exp Import = "import";
    enum_exp Export = "export"; // source export, conflict with native grammar
    enum_exp Local = "local";
};

enum_def(UniquenessLevel, std::string) {
    enum_exp document = "document";
    enum_exp project = "project";
    enum_exp group = "group";
    enum_exp scheme = "scheme";
    enum_exp global = "global";
};

enum_def(SignatureHelpTriggerKind, int)
{
    enum_exp Invoked = 1;
    enum_exp TriggerCharacter = 2;
    enum_exp ContentChange = 3;
};

enum_def(TextDocumentSaveReason, int)
{
    enum_exp Manual = 1;
    enum_exp AfterDelay = 2;
    enum_exp FocusOut = 3;
};

enum_def(TextDocumentSyncKind, int)
{
    enum_exp None = 0;
    enum_exp Full = 1;
    enum_exp Incremental = 2;
};

enum_def(NotebookCellKind, int)
{
    enum_exp Markup = 1;
    enum_exp Code = 2;
};

enum_def(DocumentHighlightKind, int){
    enum_exp Text = 1;
    enum_exp Read = 2;
    enum_exp Write = 3;
};

enum_def(CodeActionTriggerKind, int)
{
    enum_exp Invoked = 1;
    enum_exp Automatic = 2;
};

} // BasicEnum

typedef std::string DocumentUri;
typedef std::string URI ;

struct ProgressToken : std::any
{
    ProgressToken(int val) : std::any(val){}
    ProgressToken(const std::string &val) : std::any(val){}
};
std::string toJsonValueStr(const ProgressToken &val);

struct Position
{
    int line;
    int character;
    Position() = default;
    Position(int line, int character)
        : line(line), character(character){}
};
std::string toJsonValueStr(const Position &val);

struct Range
{
    Position start;
    Position end;
    Range() = default;
    Range(const Position &start, const Position &end)
        : start(start), end(end){}
};

std::string toJsonValueStr(const Range &val);

struct TextDocumentItem
{
    DocumentUri uri;
    std::string languageId;
    int version;
    std::string text;
};

struct TextDocumentIdentifier
{
    DocumentUri uri;
};
std::string toJsonValueStr(const TextDocumentIdentifier &val);

struct VersionedTextDocumentIdentifier : TextDocumentIdentifier
{
    int version;
};
std::string toJsonValueStr(const VersionedTextDocumentIdentifier &val);

struct OptionalVersionedTextDocumentIdentifier : TextDocumentIdentifier
{
    std::optional<int> version;
};
std::string toJsonValueStr(const OptionalVersionedTextDocumentIdentifier &val);

struct TextDocumentPositionParams
{
    TextDocumentIdentifier textDocument;
    Position position;
};
std::string toJsonValueStr(const TextDocumentPositionParams &val);

struct DocumentFilter
{
    std::optional<std::string> language;
    std::optional<std::string> scheme;
    std::optional<std::string> pattern;
};
std::string toJsonValueStr(const DocumentFilter &val);

struct DocumentSelector: std::vector<DocumentFilter>
{
};
std::string toJsonValueStr(const DocumentSelector &val);

struct TextEdit
{
    Range range;
    std::string newText;
};
std::string toJsonValueStr(const TextEdit &val);

struct ChangeAnnotation
{
    std::string label;
    std::optional<bool> needsConfirmation;
    std::optional<std::string> description;
};
std::string toJsonValueStr(const ChangeAnnotation &val);

typedef std::string ChangeAnnotationIdentifier;

struct AnnotatedTextEdit : TextEdit
{
    ChangeAnnotationIdentifier annotationId;
};
std::string toJsonValueStr(const AnnotatedTextEdit &val);

struct TextDocumentEdit
{
    struct Edits : std::vector<AnnotatedTextEdit>, std::vector<TextEdit> {
        Edits() = default;
        Edits(const std::vector<AnnotatedTextEdit> &val) : std::vector<AnnotatedTextEdit>(val){}
        Edits(const std::vector<TextEdit> &val) : std::vector<TextEdit>(val){}
    };
    OptionalVersionedTextDocumentIdentifier textDocument;
    Edits edits;
};
std::string toJsonValueStr(const TextDocumentEdit::Edits &val);
std::string toJsonValueStr(const TextDocumentEdit &val);

struct Location
{
    DocumentUri uri;
    Range range;
};
std::string toJsonValueStr(const Location &val);

struct LocationLink
{
    Range originSelectionRange;
    DocumentUri targetUri;
    Range targetRange;
    Range targetSelectionRange;
};

struct CodeDescription
{
    URI href;
};
std::string toJsonValueStr(const CodeDescription &val);

struct DiagnosticRelatedInformation
{
    Location location;
    std::string message;
};
std::string toJsonValueStr(const DiagnosticRelatedInformation &val);

struct Diagnostic
{
    Range range;
    std::optional<Enum::DiagnosticSeverity::type_value> severity;
    std::optional<std::any> code; // int or string
    std::optional<CodeDescription> codeDescription;
    std::optional<std::string> source;
    std::optional<std::string> message;
    std::optional<std::vector<Enum::DiagnosticTag::type_value>> tags;
    std::optional<std::vector<DiagnosticRelatedInformation>> relatedInformation;
    std::optional<std::string> data; // unknown;
};
std::string toJsonValueStr(const Diagnostic &val);

struct Command
{
    std::string title;
    std::string command;
    std::optional<std::vector<std::string>> arguments;
};
std::string toJsonValueStr(const Command &val);

struct MarkupContent
{
    Enum::MarkupKind::type_value kind;
    std::string value;
};

struct CreateFileOptions
{
    std::optional<bool> overwrite;
    std::optional<bool> ignoreIfExists;
};
std::string toJsonValueStr(const CreateFileOptions &val);

struct CreateFile
{
    const std::string kind{"create"};
    DocumentUri uri;
    std::optional<CreateFileOptions> options;
    std::optional<ChangeAnnotationIdentifier> annotationId;
};
std::string toJsonValueStr(const CreateFile &val);

struct RenameFileOptions
{
    std::optional<bool> overwrite;
    std::optional<bool> ignoreIfExists;
};
std::string toJsonValueStr(const RenameFileOptions &val);

struct RenameFile
{
    const std::string kind{"rename"};
    DocumentUri oldUri;
    DocumentUri newUri;
    std::optional<RenameFileOptions> options;
    std::optional<ChangeAnnotationIdentifier> annotationId;
};
std::string toJsonValueStr(const RenameFile &val);

struct DeleteFileOptions
{
    std::optional<bool> recursive;
    std::optional<bool> ignoreIfNotExists;
};
std::string toJsonValueStr(const DeleteFileOptions &val);

struct DeleteFile
{
    const std::string kind{"delete"};
    DocumentUri uri;
    std::optional<DeleteFileOptions> options;
    std::optional<ChangeAnnotationIdentifier> annotationId;
};
std::string toJsonValueStr(const DeleteFile &val);;

struct WorkspaceEdit
{
    // { [uri: DocumentUri]: TextEdit[]; };
    struct Changes : std::map<DocumentUri, std::vector<TextEdit>> {};
    struct ChangeAnnotations : std::map<std::string, ChangeAnnotation> {};
    struct DocumentChanges : std::any {
        DocumentChanges() = default;
        DocumentChanges(const std::vector<TextDocumentEdit> &val) : std::any(val){}
        DocumentChanges(const std::vector<CreateFile> &val) : std::any(val){}
        DocumentChanges(const std::vector<RenameFile> &val) : std::any(val){}
        DocumentChanges(const std::vector<DeleteFile> &val) : std::any(val){}
    };
    std::optional<Changes> changes;
    // ( TextDocumentEdit[] | (TextDocumentEdit | CreateFile | RenameFile | DeleteFile)[]);
    std::optional<DocumentChanges> documentChanges;
    std::optional<ChangeAnnotations> changeAnnotations;
};
std::string toJsonValueStr(const WorkspaceEdit::Changes &val);
std::string toJsonValueStr(const WorkspaceEdit::DocumentChanges &val);
std::string toJsonValueStr(const WorkspaceEdit::ChangeAnnotations &val);
std::string toJsonValueStr(const WorkspaceEdit &val);

struct WorkDoneProgressBegin
{
    const std::string kind {"begin"};
    std::string title;
    std::optional<bool> boolean;
    std::optional<std::string> message;
    std::optional<unsigned int> percentage;
};

struct WorkDoneProgressReport
{
    const std::string kind {"report"};
    std::optional<bool> cancellable;
    std::optional<std::string> message;
    std::optional<unsigned int> percentage;
};

struct WorkDoneProgressEnd
{
    const std::string kind {"end"};
    std::optional<std::string> message;
};

struct WorkDoneProgressParams
{
    std::optional<ProgressToken> workDoneToken;
};
std::string toJsonValueStr(const WorkDoneProgressParams &params);

struct WorkDoneProgressOptions
{
    std::optional<bool> workDoneProgress;
};
std::string toJsonValueStr(const WorkDoneProgressOptions &params);

struct PartialResultParams
{
    std::optional<ProgressToken> partialResultToken;
};
std::string toJsonValueStr(const PartialResultParams &params);

} // lsp

#endif // BASICJSONSTRUCTURES_H
