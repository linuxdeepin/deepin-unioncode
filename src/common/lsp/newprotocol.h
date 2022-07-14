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

#include "type/menuext.h"

#include <optional>
#include <any>
#include <vector>

namespace lsp {

struct ProgressToken {
    std::optional<int> integer;
    std::optional<std::string> string;
};

struct WorkDoneProgressParams {
    ProgressToken token;
    std::any value;
};

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
    enum_exp PlainText = "plaintext";
    enum_exp Markdown = "markdown";
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

} // BasicEnum

typedef std::string DocumentUri;
typedef std::string URI ;

struct TagSupport
{
    std::vector<BasicEnum::CompletionItemTag::type_value> valueSet;
};

struct ResolveSupport
{
    std::vector<std::string> properties;
};

struct InsertTextModeSupport
{
    std::vector<BasicEnum::InsertTextMode::type_value> valueSet;
};

struct CompletionItem
{
    std::optional<bool> snippetSupport;
    std::optional<bool> commitCharactersSupport;
    std::optional<std::vector<BasicEnum::MarkupKind::type_value>> documentationFormat;
    std::optional<bool> deprecatedSupport;
    std::optional<bool> preselectSupport;
    std::optional<TagSupport> tagSupport;
    std::optional<bool> insertReplaceSupport;
    std::optional<ResolveSupport> resolveSupport;
    std::optional<InsertTextModeSupport> insertTextModeSupport;
    std::optional<bool> labelDetailsSupport;
};

struct CompletionItemKind
{
    std::optional<std::vector<BasicEnum::CompletionItemKind>> valueSet;
};

struct CompletionList
{
    std::optional<std::vector<std::string>> itemDefaults;
};

struct CodeActionKind
{
    std::vector<BasicEnum::CodeActionKind::type_value> valueSet;
};

struct CodeActionLiteralSupport
{
    CodeActionKind codeActionKind;
};

struct ParameterInformation
{
    std::optional<bool> labelOffsetSupport;
};

struct SignatureInformation
{
    std::optional<std::vector<BasicEnum::MarkupKind::type_value>> documentationFormat;
    std::optional<ParameterInformation> parameterInformation;
    std::optional<bool> activeParameterSupport;
};

struct ChangeAnotationSupport
{
    std::optional<bool> groupsOnLabel;
};

struct SymbolKind
{
    std::vector<BasicEnum::SymbolKind::type_value> valueSet;
};

struct FoldingRangeKind
{
    std::optional<std::vector<BasicEnum::FoldingRangeKind::type_value>> valueSet;
};

struct FoldingRange
{
    std::optional<bool> collapsedText;
};


struct WorkspaceEditClientCapabilities
{
    std::optional<bool> documentChanges;
    std::optional<std::vector<BasicEnum::ResourceOperationKind::type_index>> resourceOperations;
    std::optional<BasicEnum::FailureHandlingKind::type_index> failureHandling;
    std::optional<bool> normalizesLineEndings;
    std::optional<ChangeAnotationSupport> changeAnnotationSupport;
};

struct DidChangeConfigurationClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct DidChangeWatchedFilesClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> relativePatternSupport;
};

struct ExecuteCommandClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct WorkspaceSymbolClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<std::vector<BasicEnum::SymbolKind::type_value>> symbolKind;
    std::optional<std::vector<BasicEnum::SymbolTag::type_value>> tagSupport;
    std::optional<std::vector<std::string>> resolveSupport;
    // todo
};

struct SemanticTokensWorkspaceClientCapabilities
{
    std::optional<bool> refreshSupport;
};

struct CodeLensWorkspaceClientCapabilities
{
    std::optional<bool> refreshSupport;
};

struct InlineValueWorkspaceClientCapabilities
{
    std::optional<bool> refreshSupport;
};

struct InlayHintWorkspaceClientCapabilities
{
    std::optional<bool> refreshSupport;
};

struct DiagnosticWorkspaceClientCapabilities
{
    std::optional<bool> refreshSupport;
};

struct TextDocumentSyncClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> willSave;
    std::optional<bool> willSaveWaitUntil;
    std::optional<bool> didSave;
};

struct CompletionClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<CompletionItem> completionItem;
    std::optional<CompletionItemKind> valueSet;
    std::optional<bool> contextSupport;
    std::optional<BasicEnum::InsertTextMode::type_value> insertTextMode;
    std::optional<CompletionList> itemDefaults;
};

struct HoverClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<BasicEnum::MarkupKind::type_value> contentFormat;
};

struct SignatureHelpClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<SignatureInformation> signatureInformation;
    std::optional<bool> contextSupport;
};

struct DeclarationClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> linkSupport;
};

struct DefinitionClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> linkSupport;
};

struct TypeDefinitionClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> linkSupport;
};

struct ImplementationClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> linkSupport;
};

struct ReferenceClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct DocumentHighlightClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct DocumentSymbolClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<SymbolKind> symbolKind;
    std::optional<bool> hierarchicalDocumentSymbolSupport;
    std::optional<TagSupport> tagSupport;
    std::optional<bool> labelSupport;
};

struct CodeActionClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<CodeActionLiteralSupport> codeActionLiteralSupport;
    std::optional<bool> isPreferredSupport;
    std::optional<bool> disabledSupport;
    std::optional<bool> dataSupport;
    std::optional<ResolveSupport> resolveSupport;
    std::optional<bool> honorsChangeAnnotations;
};

struct CodeLensClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct DocumentLinkClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> tooltipSupport;
};

struct DocumentColorClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct DocumentFormattingClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct DocumentRangeFormattingClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct RenameClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> prepareSupport;
    std::optional<BasicEnum::PrepareSupportDefaultBehavior> prepareSupportDefaultBehavior;
    std::optional<bool> honorsChangeAnnotations;
};

struct PublishDiagnosticsClientCapabilities
{
    std::optional<bool> relatedInformation;
    std::optional<std::vector<BasicEnum::DiagnosticTag::type_value>> tagSupport;
    std::optional<bool> versionSupport;
    std::optional<bool> codeDescriptionSupport;
    std::optional<bool> dataSupport;
};

struct FoldingRangeClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<unsigned int> rangeLimit;
    std::optional<bool> lineFoldingOnly;
    std::optional<FoldingRangeKind> foldingRangeKind;
    std::optional<FoldingRange> foldingRange;
};

struct SelectionRangeClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct LinkedEditingRangeClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct CallHierarchyClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct SemanticTokensClientCapabilities
{
    struct _Request
    {
        union _Range // boolean | {};
        {
            std::optional<bool> boolean;
            std::optional<std::any> other;
        };

        union _Full // full?: boolean | { delta?: boolean; }
        {
            std::optional<bool> boolean;
            std::optional<bool> delta;
        };

        std::optional<_Range> range;
        std::optional<_Full> full;
    };

    std::optional<bool> dynamicRegistration;
    _Request request;
    std::vector<std::string> tokenTypes;
    std::vector<std::string> tokenModifiers;
    std::vector<BasicEnum::TokenFormat::type_value> formats;
    std::optional<bool> overlappingTokenSupport;
    std::optional<bool> multilineTokenSupport;
    std::optional<bool> serverCancelSupport;
    std::optional<bool> augmentsSyntaxTokens;
};

struct MonikerClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct TypeHierarchyClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct InlineValueClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};

struct InlayHintClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<ResolveSupport> resolveSupport;
};

struct DiagnosticClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> relatedDocumentSupport;
};

struct TextDocumentClientCapabilities
{
    std::optional<TextDocumentSyncClientCapabilities> synchronization;
    std::optional<CompletionClientCapabilities> completion;
    std::optional<HoverClientCapabilities> hover;
    std::optional<SignatureHelpClientCapabilities> signatureHelp;
    std::optional<DeclarationClientCapabilities> declaration;
    std::optional<DefinitionClientCapabilities> definition;
    std::optional<TypeDefinitionClientCapabilities> typeDefinition;
    std::optional<ImplementationClientCapabilities> implementation;
    std::optional<ReferenceClientCapabilities> references;
    std::optional<DocumentHighlightClientCapabilities> documentHighlight;
    std::optional<DocumentSymbolClientCapabilities> documentSymbol;
    std::optional<CodeActionClientCapabilities> codeAction;
    std::optional<CodeLensClientCapabilities> codeLens;
    std::optional<DocumentLinkClientCapabilities> documentLink;
    std::optional<DocumentColorClientCapabilities> colorProvider;
    std::optional<DocumentFormattingClientCapabilities> formatting;
    std::optional<DocumentRangeFormattingClientCapabilities> onTypeFormatting;
    std::optional<RenameClientCapabilities> rename;
    std::optional<PublishDiagnosticsClientCapabilities> publishDiagnostics;
    std::optional<FoldingRangeClientCapabilities> foldingRange;
    std::optional<SelectionRangeClientCapabilities> selectionRange;
    std::optional<LinkedEditingRangeClientCapabilities> linkedEditingRange;
    std::optional<CallHierarchyClientCapabilities> callHierarchy;
    std::optional<SemanticTokensClientCapabilities> semanticTokens;
    std::optional<MonikerClientCapabilities> moniker;
    std::optional<TypeHierarchyClientCapabilities> typeHierarchy;
    std::optional<InlineValueClientCapabilities> inlineValue;
    std::optional<InlayHintClientCapabilities> inlayHint;
    std::optional<DiagnosticClientCapabilities> diagnostic;
};

struct FileOperations
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> didCreate;
    std::optional<bool> willCreate;
    std::optional<bool> didRename;
    std::optional<bool> willRename;
    std::optional<bool> didDelete;
    std::optional<bool> willDelete;
};

struct Workspace
{
    std::optional<bool> applyEdit;
    std::optional<WorkspaceEditClientCapabilities> workspaceEdit;
    std::optional<DidChangeConfigurationClientCapabilities> didChangeConfiguration;
    std::optional<DidChangeWatchedFilesClientCapabilities> didChangeWatchedFiles;
    std::optional<WorkspaceSymbolClientCapabilities> symbol;
    std::optional<ExecuteCommandClientCapabilities> executeCommand;
    std::optional<bool> workspaceFolders;
    std::optional<bool> configuration;
    std::optional<SemanticTokensWorkspaceClientCapabilities> semanticTokens;
    std::optional<CodeLensWorkspaceClientCapabilities> codeLens;
    std::optional<FileOperations> fileOperations;
    std::optional<InlineValueWorkspaceClientCapabilities> inlineValue;
    std::optional<InlayHintWorkspaceClientCapabilities> inlayHint;
    std::optional<DiagnosticWorkspaceClientCapabilities> diagnostics;
};

struct ClientInfo
{
    std::string name;
    std::optional<std::string> version;
};

struct NotebookDocumentSyncClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> executionSummarySupport;
};

struct NotebookDocumentClientCapabilities
{
    NotebookDocumentSyncClientCapabilities synchronization;
};

struct MessageActionItem
{
    std::optional<bool> additionalPropertiesSupport;
};

struct ShowMessageRequestClientCapabilities
{
    std::optional<MessageActionItem> messageActionItem;
};

struct ShowDocumentClientCapabilities
{
    bool support;
};

struct Window
{
    std::optional<bool> workDoneProgress;
    std::optional<ShowMessageRequestClientCapabilities> showMessage;
    std::optional<ShowDocumentClientCapabilities> showDocument;
};

struct StaleRequestSupport
{
    bool cancel;
    std::vector<std::string> retryOnContentModified;
};

struct RegularExpressionsClientCapabilities
{
    std::string engine;
    std::optional<std::string> version;
};

struct MarkdownClientCapabilities
{
    std::string parser;
    std::optional<std::string> version;
    std::optional<std::vector<std::string>> allowedTags;
};

struct General
{
    std::optional<StaleRequestSupport> staleRequestSupport;
    std::optional<RegularExpressionsClientCapabilities> regularExpressions;
    std::optional<MarkdownClientCapabilities> markdown;
    std::optional<std::vector<BasicEnum::PositionEncodingKind>> positionEncodings;
    std::optional<std::any> experimental;
};

struct ClientCapabilities
{
    std::optional<Workspace> workspace;
    std::optional<TextDocumentClientCapabilities> textDocument;
    std::optional<NotebookDocumentClientCapabilities> notebookDocument;
    std::optional<Window> window;
    std::optional<General> general;
};

struct WorkspaceFolder
{
    DocumentUri uri;
    std::string name;
};

// std::optional has default set empty is ( 'value' | null )
struct InitializeParams : WorkDoneProgressParams
{
    std::optional<int> processId{};
    std::optional<ClientInfo> clientInfo;
    std::optional<std::string> locale;
    std::optional<std::string> rootPath{};
    std::optional<std::string> rootUri{};
    std::optional<std::any> initializationOptions;
    ClientCapabilities capabilities;
    std::optional<BasicEnum::TraceValue::type_value> trace;
    std::optional<std::vector<WorkspaceFolder>> workspaceFolders{};
};
} // Initialize
} // Lifecycle
} // lsp
#endif // NEWPROTOCOL_H
