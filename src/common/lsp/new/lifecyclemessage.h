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
#ifndef LIFECYCLEMESSAHE_H
#define LIFECYCLEMESSAHE_H

#include "basicjsonstructures.h"

namespace newlsp {

struct ResolveSupport: JsonConvert
{
    std::vector<Enum::Properties::type_value> properties{};
    std::string toStdString() const;
};

struct CodeActionKind: JsonConvert
{
    std::vector<Enum::CodeActionKind::type_value> valueSet{};
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
    std::optional<std::vector<Enum::MarkupKind::type_value>> documentationFormat{};
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
    std::vector<Enum::SymbolKind::type_value> valueSet{};
    std::string toStdString() const;
};

struct FoldingRangeKind: JsonConvert
{
    std::optional<std::vector<Enum::FoldingRangeKind::type_value>> valueSet{};
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
    std::optional<std::vector<Enum::ResourceOperationKind::type_value>> resourceOperations{};
    std::optional<Enum::FailureHandlingKind::type_value> failureHandling{};
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
        std::vector<Enum::SymbolTag::type_value> valueSet{};
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
            std::vector<Enum::CompletionItemTag::type_value> valueSet{};
            std::string toStdString() const;
        };

        struct InsertTextModeSupport: JsonConvert
        {
            std::vector<Enum::InsertTextMode::type_value> valueSet{};
            std::string toStdString() const;
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
        std::string toStdString() const;
    };

    struct CompletionItemKind: JsonConvert
    {
        std::optional<std::vector<Enum::CompletionItemKind::type_value>> valueSet{};
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
    std::optional<Enum::InsertTextMode::type_value> insertTextMode{};
    std::optional<CompletionList> itemDefaults{};
    CompletionClientCapabilities() = default;
    std::string toStdString() const;
};

struct HoverClientCapabilities: JsonConvert
{
    std::optional<bool> dynamicRegistration{};
    std::optional<std::vector<Enum::MarkupKind::type_value>> contentFormat{};
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
        std::vector<Enum::SymbolKind::type_value> valueSet{};
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
    std::optional<Enum::PrepareSupportDefaultBehavior::type_value> prepareSupportDefaultBehavior{};
    std::optional<bool> honorsChangeAnnotations{};
    std::string toStdString() const;
};

struct PublishDiagnosticsClientCapabilities : JsonConvert
{
    struct TagSupport : JsonConvert
    {
        std::vector<Enum::DiagnosticTag::type_value> valueSet{};
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
    std::vector<Enum::SemanticTokenTypes::type_value> tokenTypes{};
    std::vector<Enum::SemanticTokenModifiers::type_value> tokenModifiers{};
    std::vector<Enum::TokenFormat::type_value> formats{};
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
    std::optional<std::vector<Enum::PositionEncodingKind::type_value>> positionEncodings{};
    std::optional<std::any> experimental{};
    std::string toStdString() const;
};

struct WorkspaceFolder: JsonConvert
{
    DocumentUri uri{};
    std::string name{};
    std::string toStdString() const;
};

struct ClientInfo: JsonConvert
{
    std::string name{};
    std::optional<std::string> version{};
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
    std::string toStdString() const;
};

struct InitializedParams : JsonConvert
{
    std::string toStdString() const;
};

} // namespace newlsp

#endif // LIFECYCLEMESSAHE_H
