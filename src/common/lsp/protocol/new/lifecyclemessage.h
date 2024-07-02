// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LIFECYCLEMESSAHE_H
#define LIFECYCLEMESSAHE_H

#include "basicjsonstructures.h"

namespace newlsp {

struct ResolveSupport
{
    std::vector<Enum::Properties::type_value> properties{};
};
std::string toJsonValueStr(const ResolveSupport &val);

struct CodeActionKind
{
    std::vector<Enum::CodeActionKind::type_value> valueSet{};
};
std::string toJsonValueStr(const CodeActionKind &val);

struct CodeActionLiteralSupport
{
    CodeActionKind codeActionKind{};
};
std::string toJsonValueStr(const CodeActionLiteralSupport &val);

struct ChangeAnotationSupport
{
    std::optional<bool> groupsOnLabel{};
};
std::string toJsonValueStr(const ChangeAnotationSupport &val);

struct SymbolKind
{
    std::vector<Enum::SymbolKind::type_value> valueSet{};
};
std::string toJsonValueStr(const SymbolKind &val);

struct WorkspaceEditClientCapabilities
{
    std::optional<bool> documentChanges{};
    std::optional<std::vector<Enum::ResourceOperationKind::type_value>> resourceOperations{};
    std::optional<Enum::FailureHandlingKind::type_value> failureHandling{};
    std::optional<bool> normalizesLineEndings{};
    std::optional<ChangeAnotationSupport> changeAnnotationSupport{};
};
std::string toJsonValueStr(const WorkspaceEditClientCapabilities &val);

struct DidChangeConfigurationClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const DidChangeConfigurationClientCapabilities &val);

struct DidChangeWatchedFilesClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> relativePatternSupport{};
};
std::string toJsonValueStr(const DidChangeWatchedFilesClientCapabilities &val);

struct ExecuteCommandClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const ExecuteCommandClientCapabilities &val);

struct WorkspaceSymbolClientCapabilities
{
    struct TagSupport
    {
        std::vector<Enum::SymbolTag::type_value> valueSet{};
    };
    std::optional<bool> dynamicRegistration{};
    std::optional<SymbolKind> symbolKind{};
    std::optional<TagSupport> tagSupport{};
    std::optional<std::vector<std::string>> resolveSupport{};

};
std::string toJsonValueStr(const WorkspaceSymbolClientCapabilities &val);

struct SemanticTokensWorkspaceClientCapabilities
{
    std::optional<bool> refreshSupport{};
};
std::string toJsonValueStr(const SemanticTokensWorkspaceClientCapabilities &val);

struct CodeLensWorkspaceClientCapabilities
{
    std::optional<bool> refreshSupport{};
};
std::string toJsonValueStr(const CodeLensWorkspaceClientCapabilities &val);

struct InlineValueWorkspaceClientCapabilities
{
    std::optional<bool> refreshSupport{};
};
std::string toJsonValueStr(const InlineValueWorkspaceClientCapabilities &val);

struct InlayHintWorkspaceClientCapabilities
{
    std::optional<bool> refreshSupport{};
};
std::string toJsonValueStr(const InlayHintWorkspaceClientCapabilities &val);

struct DiagnosticWorkspaceClientCapabilities
{
    std::optional<bool> refreshSupport{};
};
std::string toJsonValueStr(const DiagnosticWorkspaceClientCapabilities &val);

struct TextDocumentSyncClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> willSave{};
    std::optional<bool> willSaveWaitUntil{};
    std::optional<bool> didSave{};
};
std::string toJsonValueStr(const TextDocumentSyncClientCapabilities &val);

struct CompletionClientCapabilities
{
    struct CompletionItem
    {
        struct TagSupport
        {
            std::vector<Enum::CompletionItemTag::type_value> valueSet{};
        };

        struct InsertTextModeSupport
        {
            std::vector<Enum::InsertTextMode::type_value> valueSet{};
        };

        std::optional<bool> snippetSupport{};
        std::optional<bool> commitCharactersSupport{};
        std::optional<std::vector<Enum::MarkupKind::type_value>> documentationFormat{};
        std::optional<bool> deprecatedSupport{};
        std::optional<bool> preselectSupport{};
        std::optional<TagSupport> tagSupport{};
        std::optional<bool> insertReplaceSupport{};
        std::optional<ResolveSupport> resolveSupport{};
        std::optional<InsertTextModeSupport> insertTextModeSupport{};
        std::optional<bool> labelDetailsSupport{};
        CompletionItem() = default;

    };

    struct CompletionItemKind
    {
        std::optional<std::vector<Enum::CompletionItemKind::type_value>> valueSet{};
        CompletionItemKind() = default;
    };

    struct CompletionList
    {
        std::optional<std::vector<std::string>> itemDefaults{};
    };

    std::optional<bool> dynamicRegistration{};
    std::optional<CompletionItem> completionItem{};
    std::optional<CompletionItemKind> completionItemKind{};
    std::optional<bool> contextSupport{};
    std::optional<Enum::InsertTextMode::type_value> insertTextMode{};
    std::optional<CompletionList> itemDefaults{};
    CompletionClientCapabilities() = default;
};
std::string toJsonValueStr(const CompletionClientCapabilities &val);
std::string toJsonValueStr(const CompletionClientCapabilities::CompletionItem &val);
std::string toJsonValueStr(const CompletionClientCapabilities::CompletionItem::TagSupport &val);
std::string toJsonValueStr(const CompletionClientCapabilities::CompletionItem::InsertTextModeSupport &val);
std::string toJsonValueStr(const CompletionClientCapabilities::CompletionList &val);
std::string toJsonValueStr(const CompletionClientCapabilities::CompletionItemKind &val);

struct HoverClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<std::vector<Enum::MarkupKind::type_value>> contentFormat{};
};
std::string toJsonValueStr(const HoverClientCapabilities &val);

struct SignatureHelpClientCapabilities
{
    struct SignatureInformation
    {
        struct ParameterInformation
        {
            std::optional<bool> labelOffsetSupport{};
        };
        std::optional<std::vector<Enum::MarkupKind::type_value>> documentationFormat{};
        std::optional<ParameterInformation> parameterInformation{};
        std::optional<bool> activeParameterSupport{};
    };
    std::optional<bool> dynamicRegistration{};
    std::optional<SignatureInformation> signatureInformation{};
    std::optional<bool> contextSupport{};
};
std::string toJsonValueStr(const SignatureHelpClientCapabilities::SignatureInformation::ParameterInformation &val);
std::string toJsonValueStr(const SignatureHelpClientCapabilities::SignatureInformation &val);
std::string toJsonValueStr(const SignatureHelpClientCapabilities &val);

struct DeclarationClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> linkSupport{};
};
std::string toJsonValueStr(const DeclarationClientCapabilities &val);

struct DefinitionClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> linkSupport{};
};
std::string toJsonValueStr(const DefinitionClientCapabilities &val);

struct TypeDefinitionClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> linkSupport{};
};
std::string toJsonValueStr(const TypeDefinitionClientCapabilities &val);

struct ImplementationClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> linkSupport{};
};
std::string toJsonValueStr(const ImplementationClientCapabilities &val);

struct ReferenceClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const ReferenceClientCapabilities &val);

struct DocumentHighlightClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const DocumentHighlightClientCapabilities &val);

struct DocumentSymbolClientCapabilities
{
    struct TagSupport
    {
        std::vector<Enum::SymbolKind::type_value> valueSet{};
    };
    std::optional<bool> dynamicRegistration{};
    std::optional<SymbolKind> symbolKind{};
    std::optional<bool> hierarchicalDocumentSymbolSupport{};
    std::optional<TagSupport> tagSupport{};
    std::optional<bool> labelSupport{};

};
std::string toJsonValueStr(const DocumentSymbolClientCapabilities &val);

struct CodeActionClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<CodeActionLiteralSupport> codeActionLiteralSupport{};
    std::optional<bool> isPreferredSupport{};
    std::optional<bool> disabledSupport{};
    std::optional<bool> dataSupport{};
    std::optional<ResolveSupport> resolveSupport{};
    std::optional<bool> honorsChangeAnnotations{};
};
std::string toJsonValueStr(const CodeActionClientCapabilities &val);

struct CodeLensClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const CodeLensClientCapabilities &val);

struct DocumentLinkClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> tooltipSupport{};
};
std::string toJsonValueStr(const DocumentLinkClientCapabilities &val);

struct DocumentColorClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const DocumentColorClientCapabilities &val);

struct DocumentFormattingClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const DocumentFormattingClientCapabilities &val);

struct DocumentRangeFormattingClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const DocumentRangeFormattingClientCapabilities &val);

struct DocumentOnTypeFormattingClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const DocumentOnTypeFormattingClientCapabilities &val);

struct RenameClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> prepareSupport{};
    std::optional<Enum::PrepareSupportDefaultBehavior::type_value> prepareSupportDefaultBehavior{};
    std::optional<bool> honorsChangeAnnotations{};
};
std::string toJsonValueStr(const RenameClientCapabilities &val);

struct PublishDiagnosticsClientCapabilities
{
    struct TagSupport
    {
        std::vector<Enum::DiagnosticTag::type_value> valueSet{};
    };
    std::optional<bool> relatedInformation{};
    std::optional<TagSupport> tagSupport{};
    std::optional<bool> versionSupport{};
    std::optional<bool> codeDescriptionSupport{};
    std::optional<bool> dataSupport{};
};
std::string toJsonValueStr(const PublishDiagnosticsClientCapabilities &val);

struct FoldingRangeClientCapabilities
{
    struct FoldingRangeKind
    {
        std::optional<std::vector<Enum::FoldingRangeKind::type_value>> valueSet{};
    };

    struct FoldingRange
    {
        std::optional<bool> collapsedText{};
    };
    std::optional<bool> dynamicRegistration{};
    std::optional<unsigned int> rangeLimit{};
    std::optional<bool> lineFoldingOnly{};
    std::optional<FoldingRangeKind> foldingRangeKind{};
    std::optional<FoldingRange> foldingRange;
};
std::string toJsonValueStr(const FoldingRangeClientCapabilities::FoldingRange &val);
std::string toJsonValueStr(const FoldingRangeClientCapabilities::FoldingRangeKind &val);
std::string toJsonValueStr(const FoldingRangeClientCapabilities &val);

struct SelectionRangeClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const SelectionRangeClientCapabilities &val);

struct LinkedEditingRangeClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const LinkedEditingRangeClientCapabilities &val);

struct CallHierarchyClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const CallHierarchyClientCapabilities &val);

struct SemanticTokensClientCapabilities
{
    struct Requests
    {
        struct Full
        {
            std::optional<bool> delta;
        };

        std::optional<std::any> range{}; // boolean | {};
        std::optional<Full> full{}; // full?: boolean | { delta?: boolean; }
    };

    Requests requests;
    std::vector<Enum::SemanticTokenTypes::type_value> tokenTypes{};
    std::vector<Enum::SemanticTokenModifiers::type_value> tokenModifiers{};
    std::vector<Enum::TokenFormat::type_value> formats{};
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> overlappingTokenSupport{};
    std::optional<bool> multilineTokenSupport{};
    std::optional<bool> serverCancelSupport{};
    std::optional<bool> augmentsSyntaxTokens{};
};
std::string toJsonValueStr(const SemanticTokensClientCapabilities &val);

struct MonikerClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const MonikerClientCapabilities &val);

struct TypeHierarchyClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const TypeHierarchyClientCapabilities &val);

struct InlineValueClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
};
std::string toJsonValueStr(const InlineValueClientCapabilities &val);

struct InlayHintClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<ResolveSupport> resolveSupport{};
};
std::string toJsonValueStr(const InlayHintClientCapabilities &val);

struct DiagnosticClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> relatedDocumentSupport{};
};
std::string toJsonValueStr(const DiagnosticClientCapabilities &val);

struct TextDocumentClientCapabilities
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
};
std::string toJsonValueStr(const TextDocumentClientCapabilities &val);

struct FileOperations
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> didCreate{};
    std::optional<bool> willCreate{};
    std::optional<bool> didRename{};
    std::optional<bool> willRename{};
    std::optional<bool> didDelete{};
    std::optional<bool> willDelete{};
};
std::string toJsonValueStr(const FileOperations &val);

struct Workspace
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
};
std::string toJsonValueStr(const Workspace &val);

struct NotebookDocumentSyncClientCapabilities
{
    std::optional<bool> dynamicRegistration{};
    std::optional<bool> executionSummarySupport{};
};
std::string toJsonValueStr(const NotebookDocumentSyncClientCapabilities &val);

struct NotebookDocumentClientCapabilities
{
    NotebookDocumentSyncClientCapabilities synchronization{};
};
std::string toJsonValueStr(const NotebookDocumentClientCapabilities &val);

struct MessageActionItem
{
    std::optional<bool> additionalPropertiesSupport{};
};
std::string toJsonValueStr(const MessageActionItem &val);

struct ShowMessageRequestClientCapabilities
{
    std::optional<MessageActionItem> messageActionItem{};
};
std::string toJsonValueStr(const ShowMessageRequestClientCapabilities &val);

struct ShowDocumentClientCapabilities
{
    bool support{};
};
std::string toJsonValueStr(const ShowDocumentClientCapabilities &val);

struct Window
{
    std::optional<bool> workDoneProgress{};
    std::optional<ShowMessageRequestClientCapabilities> showMessage{};
    std::optional<ShowDocumentClientCapabilities> showDocument{};
};
std::string toJsonValueStr(const Window &val);

struct StaleRequestSupport
{
    bool cancel{};
    std::vector<std::string> retryOnContentModified{};
};
std::string toJsonValueStr(const StaleRequestSupport &val);

struct RegularExpressionsClientCapabilities
{
    std::string engine{};
    std::optional<std::string> version{};
};
std::string toJsonValueStr(const RegularExpressionsClientCapabilities &val);

struct MarkdownClientCapabilities
{
    std::string parser{};
    std::optional<std::string> version{};
    std::optional<std::vector<std::string>> allowedTags{};
};
std::string toJsonValueStr(const MarkdownClientCapabilities &val);

struct General
{
    std::optional<StaleRequestSupport> staleRequestSupport{};
    std::optional<RegularExpressionsClientCapabilities> regularExpressions{};
    std::optional<MarkdownClientCapabilities> markdown{};
    std::optional<std::vector<Enum::PositionEncodingKind::type_value>> positionEncodings{};
    std::optional<std::any> experimental{};
};
std::string toJsonValueStr(const General &val);

struct WorkspaceFolder
{
    DocumentUri uri{};
    std::string name{};
};
std::string toJsonValueStr(const WorkspaceFolder &val);
std::string toJsonValueStr(const std::vector<WorkspaceFolder> &val);

struct ClientInfo
{
    std::string name{};
    std::optional<std::string> version{};
};
std::string toJsonValueStr(const ClientInfo &val);

struct ClientCapabilities
{
    std::optional<Workspace> workspace{};
    std::optional<TextDocumentClientCapabilities> textDocument{};
    std::optional<NotebookDocumentClientCapabilities> notebookDocument{};
    std::optional<Window> window{};
    std::optional<General> general{};
};
std::string toJsonValueStr(const ClientCapabilities &val);

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
    std::optional<Enum::TraceValue::type_value> trace{};
    std::optional<std::vector<WorkspaceFolder>> workspaceFolders{};
    std::string formatValue(const std::vector<WorkspaceFolder> &workspaceFolders) const;
};
std::string toJsonValueStr(const InitializeParams &val);

struct InitializedParams
{

};
std::string toJsonValueStr(const InitializedParams &val);

} // namespace newlsp

#endif // LIFECYCLEMESSAHE_H
