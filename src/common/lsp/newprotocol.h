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
#ifndef NEWPROTOCOL_H
#define NEWPROTOCOL_H

#include "common/type/menuext.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

#include <iostream>
#include <vector>
#include <optional>
#include <any>

#include <QDebug>
namespace newlsp {

template<class T>
static bool any_contrast(const std::any &any)
{
    if (any.type() == std::any(T()).type()) {
        return true;
    }
    return false;
}

typedef std::string DocumentUri;
typedef std::string URI ;
typedef std::any ProgressToken; // integer | string;

namespace Lifecycle {
namespace Initialize {
namespace BasicEnum {

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

} // BasicEnum

struct JsonConvert
{
    static std::string formatScope(const std::string &src);
    static std::string formatKey(const std::string &key);
    static std::string formatValue(unsigned int value);
    static std::string formatValue(int value);
    static std::string formatValue(bool value);
    static std::string formatValue(const std::string &value);
    static std::string formatValue(const std::vector<int> &vecInt);
    static std::string formatValue(const std::vector<std::string> &vecString);

    static std::string addValue(const std::string &src,
                                const std::pair<std::string, std::any> &elem);

    static std::string addValue(const std::string &src,
                                std::initializer_list<std::pair<std::string, std::any>> &elems);
};

struct WorkDoneProgressParams: JsonConvert
{
    std::optional<ProgressToken> workDoneToken{};
    std::string toStdString() const;
};

struct ResolveSupport: JsonConvert
{
    std::vector<BasicEnum::Properties::type_value> properties{};
    std::string toStdString() const;
};

struct CodeActionKind: JsonConvert
{
    std::vector<BasicEnum::CodeActionKind::type_value> valueSet{};
    std::string toStdString() const;
};

struct CodeActionLiteralSupport: JsonConvert
{
    CodeActionKind codeActionKind{};
    std::string toStdString() const;
};

struct ParameterInformation: JsonConvert
{
    std::optional<bool> labelOffsetSupport{};
    std::string toStdString() const;
};

struct SignatureInformation: JsonConvert
{
    std::optional<std::vector<BasicEnum::MarkupKind::type_value>> documentationFormat{};
    std::optional<ParameterInformation> parameterInformation{};
    std::optional<bool> activeParameterSupport{};
    std::string toStdString() const;
};

struct ChangeAnotationSupport: JsonConvert
{
    std::optional<bool> groupsOnLabel{};
    std::string toStdString() const;
};

struct SymbolKind: JsonConvert
{
    std::vector<BasicEnum::SymbolKind::type_value> valueSet{};
    std::string toStdString() const;
};

struct FoldingRangeKind: JsonConvert
{
    std::optional<std::vector<BasicEnum::FoldingRangeKind::type_value>> valueSet{};
    std::string toStdString() const;
};

struct FoldingRange: JsonConvert
{
    std::optional<bool> collapsedText{};
    std::string toStdString() const;
};

struct WorkspaceEditClientCapabilities: JsonConvert
{
    std::optional<bool> documentChanges{};
    std::optional<std::vector<BasicEnum::ResourceOperationKind::type_value>> resourceOperations{};
    std::optional<BasicEnum::FailureHandlingKind::type_value> failureHandling{};
    std::optional<bool> normalizesLineEndings{};
    std::optional<ChangeAnotationSupport> changeAnnotationSupport{};
    std::string toStdString() const;
};

struct DidChangeConfigurationClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct DidChangeWatchedFilesClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> relativePatternSupport{};
    std::string toStdString() const;
};

struct ExecuteCommandClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct WorkspaceSymbolClientCapabilities: JsonConvert
{
    struct TagSupport : JsonConvert
    {
        std::vector<BasicEnum::SymbolTag::type_value> valueSet{};
        std::string toStdString() const;
    };
    std::optional<bool> dynamicRegistration{};
    std::optional<SymbolKind> symbolKind{};
    std::optional<TagSupport> tagSupport{};
    std::optional<std::vector<std::string>> resolveSupport{};
    std::string toStdString() const;
};

struct SemanticTokensWorkspaceClientCapabilities: JsonConvert
{
    std::optional<bool> refreshSupport{};
    std::string toStdString() const;
};

struct CodeLensWorkspaceClientCapabilities: JsonConvert
{
    std::optional<bool> refreshSupport{};
    std::string toStdString() const;
};

struct InlineValueWorkspaceClientCapabilities: JsonConvert
{
    std::optional<bool> refreshSupport{};
    std::string toStdString() const;
};

struct InlayHintWorkspaceClientCapabilities: JsonConvert
{
    std::optional<bool> refreshSupport{};
    std::string toStdString() const;
};

struct DiagnosticWorkspaceClientCapabilities: JsonConvert
{
    std::optional<bool> refreshSupport{};
    std::string toStdString() const;
};

struct TextDocumentSyncClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> willSave{};
    std::optional<bool> willSaveWaitUntil{};
    std::optional<bool> didSave{};
    std::string toStdString() const;
};

struct CompletionClientCapabilities: JsonConvert
{
    struct CompletionItem: JsonConvert
    {
        struct TagSupport: JsonConvert
        {
            std::vector<BasicEnum::CompletionItemTag::type_value> valueSet{};
            std::string toStdString() const;
        };

        struct InsertTextModeSupport: JsonConvert
        {
            std::vector<BasicEnum::InsertTextMode::type_value> valueSet{};
            std::string toStdString() const;
        };

        std::optional<bool> snippetSupport{};
        std::optional<bool> commitCharactersSupport{};
        std::optional<std::vector<BasicEnum::MarkupKind::type_value>> documentationFormat{};
        std::optional<bool> deprecatedSupport{};
        std::optional<bool> preselectSupport{};
        std::optional<TagSupport> tagSupport{};
        std::optional<bool> insertReplaceSupport{};
        std::optional<ResolveSupport> resolveSupport{};
        std::optional<InsertTextModeSupport> insertTextModeSupport{};
        std::optional<bool> labelDetailsSupport{};
        CompletionItem() = default;
        std::string toStdString() const;
    };

    struct CompletionItemKind: JsonConvert
    {
        std::optional<std::vector<BasicEnum::CompletionItemKind::type_value>> valueSet{};
        CompletionItemKind() = default;
        std::string toStdString() const;
    };

    struct CompletionList: JsonConvert
    {
        std::optional<std::vector<std::string>> itemDefaults{};
        std::string toStdString() const;
    };

    std::optional<bool> dynamicRegistration{};
    std::optional<CompletionItem> completionItem{};
    std::optional<CompletionItemKind> completionItemKind{};
    std::optional<bool> contextSupport{};
    std::optional<BasicEnum::InsertTextMode::type_value> insertTextMode{};
    std::optional<CompletionList> itemDefaults{};
    CompletionClientCapabilities() = default;
    std::string toStdString() const;
};

struct HoverClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<std::vector<BasicEnum::MarkupKind::type_value>> contentFormat{};
    std::string toStdString() const;
};

struct SignatureHelpClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<SignatureInformation> signatureInformation{};
    std::optional<bool> contextSupport{};
    std::string toStdString() const;
};

struct DeclarationClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> linkSupport{};
    std::string toStdString() const;
};

struct DefinitionClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> linkSupport{};
    std::string toStdString() const;
};

struct TypeDefinitionClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> linkSupport{};
    std::string toStdString() const;
};

struct ImplementationClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> linkSupport{};
    std::string toStdString() const;
};

struct ReferenceClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct DocumentHighlightClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct DocumentSymbolClientCapabilities : JsonConvert
{
    struct TagSupport : JsonConvert
    {
        std::vector<BasicEnum::SymbolKind::type_value> valueSet{};
        std::string toStdString() const;
    };
    std::optional<bool> dynamicRegistration{};
    std::optional<SymbolKind> symbolKind{};
    std::optional<bool> hierarchicalDocumentSymbolSupport{};
    std::optional<TagSupport> tagSupport{};
    std::optional<bool> labelSupport{};
    std::string toStdString() const;
};

struct CodeActionClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<CodeActionLiteralSupport> codeActionLiteralSupport{};
    std::optional<bool> isPreferredSupport{};
    std::optional<bool> disabledSupport{};
    std::optional<bool> dataSupport{};
    std::optional<ResolveSupport> resolveSupport{};
    std::optional<bool> honorsChangeAnnotations{};
    std::string toStdString() const;
};

struct CodeLensClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct DocumentLinkClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> tooltipSupport{};
    std::string toStdString() const;
};

struct DocumentColorClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct DocumentFormattingClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct DocumentRangeFormattingClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct DocumentOnTypeFormattingClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct RenameClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> prepareSupport{};
    std::optional<BasicEnum::PrepareSupportDefaultBehavior::type_value> prepareSupportDefaultBehavior{};
    std::optional<bool> honorsChangeAnnotations{};
    std::string toStdString() const;
};

struct PublishDiagnosticsClientCapabilities : JsonConvert
{
    struct TagSupport : JsonConvert
    {
        std::vector<BasicEnum::DiagnosticTag::type_value> valueSet{};
        std::string toStdString() const;
    };
    std::optional<bool> relatedInformation{};
    std::optional<TagSupport> tagSupport{};
    std::optional<bool> versionSupport{};
    std::optional<bool> codeDescriptionSupport{};
    std::optional<bool> dataSupport{};
    std::string toStdString() const;
};

struct FoldingRangeClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<unsigned int> rangeLimit{};
    std::optional<bool> lineFoldingOnly{};
    std::optional<FoldingRangeKind> foldingRangeKind{};
    std::optional<FoldingRange> foldingRange;
    std::string toStdString() const;
};

struct SelectionRangeClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct LinkedEditingRangeClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct CallHierarchyClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct SemanticTokensClientCapabilities : JsonConvert
{
    struct Requests : JsonConvert
    {
        struct Full : JsonConvert
        {
            std::optional<bool> delta;
            std::string toStdString() const;
        };

        std::optional<std::any> range{}; // boolean | {};
        std::optional<Full> full{}; // full?: boolean | { delta?: boolean; }
        std::string toStdString() const;
    };

    Requests requests;
    std::vector<BasicEnum::SemanticTokenTypes::type_value> tokenTypes{};
    std::vector<BasicEnum::SemanticTokenModifiers::type_value> tokenModifiers{};
    std::vector<BasicEnum::TokenFormat::type_value> formats{};
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> overlappingTokenSupport{};
    std::optional<bool> multilineTokenSupport{};
    std::optional<bool> serverCancelSupport{};
    std::optional<bool> augmentsSyntaxTokens{};
    std::string toStdString() const;
};

struct MonikerClientCapabilities : JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct TypeHierarchyClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct InlineValueClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::string toStdString() const;
};

struct InlayHintClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<ResolveSupport> resolveSupport{};
    std::string toStdString() const;
};

struct DiagnosticClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> relatedDocumentSupport{};
    std::string toStdString() const;
};

struct TextDocumentClientCapabilities: JsonConvert
{
    std::optional<TextDocumentSyncClientCapabilities> synchronization{};
    std::optional<CompletionClientCapabilities> completion{};
    std::optional<HoverClientCapabilities> hover{};
    std::optional<SignatureHelpClientCapabilities> signatureHelp{};
    std::optional<DeclarationClientCapabilities> declaration{};
    std::optional<DefinitionClientCapabilities> definition{};
    std::optional<TypeDefinitionClientCapabilities> typeDefinition{};
    std::optional<ImplementationClientCapabilities> implementation{};
    std::optional<ReferenceClientCapabilities> references{};
    std::optional<DocumentHighlightClientCapabilities> documentHighlight{};
    std::optional<DocumentSymbolClientCapabilities> documentSymbol{};
    std::optional<CodeActionClientCapabilities> codeAction{};
    std::optional<CodeLensClientCapabilities> codeLens{};
    std::optional<DocumentLinkClientCapabilities> documentLink{};
    std::optional<DocumentColorClientCapabilities> colorProvider{};
    std::optional<DocumentFormattingClientCapabilities> formatting{};
    std::optional<DocumentRangeFormattingClientCapabilities> rangeFormatting{};
    std::optional<DocumentOnTypeFormattingClientCapabilities> onTypeFormatting{};
    std::optional<RenameClientCapabilities> rename{};
    std::optional<PublishDiagnosticsClientCapabilities> publishDiagnostics{};
    std::optional<FoldingRangeClientCapabilities> foldingRange{};
    std::optional<SelectionRangeClientCapabilities> selectionRange{};
    std::optional<LinkedEditingRangeClientCapabilities> linkedEditingRange{};
    std::optional<CallHierarchyClientCapabilities> callHierarchy{};
    std::optional<SemanticTokensClientCapabilities> semanticTokens{};
    std::optional<MonikerClientCapabilities> moniker{};
    std::optional<TypeHierarchyClientCapabilities> typeHierarchy{};
    std::optional<InlineValueClientCapabilities> inlineValue{};
    std::optional<InlayHintClientCapabilities> inlayHint{};
    std::optional<DiagnosticClientCapabilities> diagnostic{};
    std::string toStdString() const;
};

struct FileOperations: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> didCreate{};
    std::optional<bool> willCreate{};
    std::optional<bool> didRename{};
    std::optional<bool> willRename{};
    std::optional<bool> didDelete{};
    std::optional<bool> willDelete{};
    std::string toStdString() const;
};

struct Workspace: JsonConvert
{
    std::optional<bool> applyEdit{};
    std::optional<WorkspaceEditClientCapabilities> workspaceEdit{};
    std::optional<DidChangeConfigurationClientCapabilities> didChangeConfiguration{};
    std::optional<DidChangeWatchedFilesClientCapabilities> didChangeWatchedFiles{};
    std::optional<WorkspaceSymbolClientCapabilities> symbol{};
    std::optional<ExecuteCommandClientCapabilities> executeCommand{};
    std::optional<bool> workspaceFolders{};
    std::optional<bool> configuration{};
    std::optional<SemanticTokensWorkspaceClientCapabilities> semanticTokens{};
    std::optional<CodeLensWorkspaceClientCapabilities> codeLens{};
    std::optional<FileOperations> fileOperations{};
    std::optional<InlineValueWorkspaceClientCapabilities> inlineValue{};
    std::optional<InlayHintWorkspaceClientCapabilities> inlayHint{};
    std::optional<DiagnosticWorkspaceClientCapabilities> diagnostics{};
    std::string toStdString() const;
};

struct ClientInfo: JsonConvert
{
    std::string name{};
    std::optional<std::string> version{};
    std::string toStdString() const;
};

struct NotebookDocumentSyncClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> executionSummarySupport{};
    std::string toStdString() const;
};

struct NotebookDocumentClientCapabilities: JsonConvert
{
    NotebookDocumentSyncClientCapabilities synchronization{};
    std::string toStdString() const;
};

struct MessageActionItem: JsonConvert
{
    std::optional<bool> additionalPropertiesSupport{};
    std::string toStdString() const;
};

struct ShowMessageRequestClientCapabilities: JsonConvert
{
    std::optional<MessageActionItem> messageActionItem{};
    std::string toStdString() const;
};

struct ShowDocumentClientCapabilities: JsonConvert
{
    bool support{};
    std::string toStdString() const;
};

struct Window: JsonConvert
{
    std::optional<bool> workDoneProgress{};
    std::optional<ShowMessageRequestClientCapabilities> showMessage{};
    std::optional<ShowDocumentClientCapabilities> showDocument{};
    std::string toStdString() const;
};

struct StaleRequestSupport: JsonConvert
{
    bool cancel{};
    std::vector<std::string> retryOnContentModified{};
    std::string toStdString() const;
};

struct RegularExpressionsClientCapabilities: JsonConvert
{
    std::string engine{};
    std::optional<std::string> version{};
    std::string toStdString() const;
};

struct MarkdownClientCapabilities: JsonConvert
{
    std::string parser{};
    std::optional<std::string> version{};
    std::optional<std::vector<std::string>> allowedTags{};
    std::string toStdString() const;
};

struct General: JsonConvert
{
    std::optional<StaleRequestSupport> staleRequestSupport{};
    std::optional<RegularExpressionsClientCapabilities> regularExpressions{};
    std::optional<MarkdownClientCapabilities> markdown{};
    std::optional<std::vector<BasicEnum::PositionEncodingKind::type_value>> positionEncodings{};
    std::optional<std::any> experimental{};
    std::string toStdString() const;
};

struct ClientCapabilities: JsonConvert
{
    std::optional<Workspace> workspace{};
    std::optional<TextDocumentClientCapabilities> textDocument{};
    std::optional<NotebookDocumentClientCapabilities> notebookDocument{};
    std::optional<Window> window{};
    std::optional<General> general{};
    std::string toStdString() const;
};

struct WorkspaceFolder: JsonConvert
{
    DocumentUri uri{};
    std::string name{};
    std::string toStdString() const;
};

// std::optional has default set empty is ( 'value' | null )
struct InitializeParams : WorkDoneProgressParams
{
    std::optional<int> processId{};
    std::optional<ClientInfo> clientInfo{};
    std::optional<std::string> locale{};
    std::optional<std::string> rootPath{};
    std::optional<std::string> rootUri{};
    std::optional<std::string> language{}; // extend
    std::optional<std::any> initializationOptions{};
    ClientCapabilities capabilities{};
    std::optional<BasicEnum::TraceValue::type_value> trace{};
    std::optional<std::vector<WorkspaceFolder>> workspaceFolders{};
    std::string formatValue(const std::vector<WorkspaceFolder> &workspaceFolders) const;
    std::string toStdString() const;
};
} // Initialize
} // Lifecycle

typedef std::string ChangeAnnotationIdentifier;

struct Position{
    int line;
    int character;
};
struct Range
{
    Position start;
    Position end;
};

struct TextEdit {
    Range range;
    std::string newText;
};

struct AnnotatedTextEdit : TextEdit
{
    ChangeAnnotationIdentifier annotationId;
};

struct TextDocumentIdentifier
{
    DocumentUri uri;
};

struct OptionalVersionedTextDocumentIdentifier : TextDocumentIdentifier
{
    std::optional<int> version;
};

struct TextDocumentEdit
{
    OptionalVersionedTextDocumentIdentifier textDocument;
    std::vector<AnnotatedTextEdit> edits;
};

namespace Workspace
{
struct CreateFileOptions {
    std::optional<bool> overwrite;
    std::optional<bool> ignoreIfExists;
};

struct RenameFileOptions {
    std::optional<bool> overwrite;
    std::optional<bool> ignoreIfExists;
};

struct CreateFile
{
    std::string kind{"create"};
    DocumentUri uri;
    std::optional<CreateFileOptions> options;
    std::optional<ChangeAnnotationIdentifier> annotationId;
};

struct RenameFile
{
    std::string kind{"rename"};
    DocumentUri oldUri;
    DocumentUri newUri;
    std::optional<RenameFileOptions> options;
    std::optional<ChangeAnnotationIdentifier> annotationId;
};

struct DeleteFileOptions
{
    std::optional<bool> recursive;
    std::optional<bool> ignoreIfNotExists;
};

struct DeleteFile
{
    std::string kind{"delete"};
    DocumentUri uri;
    std::optional<DeleteFileOptions> options;
    std::optional<ChangeAnnotationIdentifier> annotationId;
};

struct ChangeAnnotation
{
    std::string label;
    bool needsConfirmation;
    std::optional<std::string> description;
};

struct WorkspaceEdit
{
    // { [uri: DocumentUri]: TextEdit[]; };
    std::optional<std::map<DocumentUri, std::vector<TextEdit>>> changes;
    // ( TextDocumentEdit[] | (TextDocumentEdit | CreateFile | RenameFile | DeleteFile)[]);
    std::optional<std::any> documentChanges;
    std::optional<std::map<std::string, ChangeAnnotation>> changeAnnotations;
};
}
} // newlsp
#endif // NEWPROTOCOL_H
