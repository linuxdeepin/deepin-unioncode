// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LANGUAGEFEATURES_H
#define LANGUAGEFEATURES_H

#include "basicjsonstructures.h"

namespace newlsp {

/** Goto Declaration Request
 * @brief The DeclarationParams struct
 * Request:
 *      method: textDocument/declaration
 *      params: DeclarationParams defined as follows:
 * Response:
 *      result: Location | Location[] | LocationLink[] |null
 *      partial result: Location[] | LocationLink[]
 *      error: code and message set in case an exception happens during the declaration request.
 */
struct DeclarationParams : TextDocumentPositionParams,
        WorkDoneProgressParams, PartialResultParams {
};
std::string toJsonValueStr(const DeclarationParams &val);

/**
 * @brief The DefinitionParams struct
 * Request:
 *      method: textDocument/definition
 *      params: DefinitionParams defined as follows:
 * Response:
 *      result: Location | Location[] | LocationLink[] | null
 *      partial result: Location[] | LocationLink[]
 *      error: code and message set in case an exception happens during the definition request.
 */
struct DefinitionParams : TextDocumentPositionParams,
        WorkDoneProgressParams, PartialResultParams {
};
std::string toJsonValueStr(const DefinitionParams &val);

/** Goto Type Definition Request
 * @brief The TypeDefinitionParams struct
 * Request:
 *      method: textDocument/typeDefinition
 *      params: TypeDefinitionParams defined as follows:
 * Response:
 *      result: Location | Location[] | LocationLink[] | null
 *      partial result: Location[] | LocationLink[]
 *      error: code and message set in case an exception happens during the definition request.
 */
struct TypeDefinitionParams : TextDocumentPositionParams,
        WorkDoneProgressParams, PartialResultParams {
};
std::string toJsonValueStr(const TypeDefinitionParams &val);

/** Goto Implementation Request
 * @brief The ImplementationParams struct
 * Request:
 *      method: textDocument/implementation
 *      params: ImplementationParams defined as follows:
 * Response:
 *      result: Location | Location[] | LocationLink[] | null
 *      partial result: Location[] | LocationLink[]
 *      error: code and message set in case an exception happens during the definition request.
 */
struct ImplementationParams : TextDocumentPositionParams,
        WorkDoneProgressParams, PartialResultParams {
};
std::string toJsonValueStr(const ImplementationParams &val);

/** Find References Request
 * @brief The ReferenceParams struct
 * Request:
 *      method: textDocument/references
 *      params: ReferenceParams defined
 * Response:
 *      result: Location[] | null
 *      partial result: Location[]
 *      error: code and message set in case an exception happens during the reference request.
 */
struct ReferenceParams : TextDocumentPositionParams,
        WorkDoneProgressParams, PartialResultParams
{
    struct ReferenceContext
    {
        bool includeDeclaration;
    };
    ReferenceContext context;
};
std::string toJsonValueStr(const ReferenceParams::ReferenceContext &val);
std::string toJsonValueStr(const ReferenceParams &val);

/** namespace { Prepare Call Hierarchy Request
 * @brief The CallHierarchyPrepareParams struct
 * Request:
 *      method: textDocument/prepareCallHierarchy
 *      params: CallHierarchyPrepareParams defined
 * Response:
 *      result: CallHierarchyItem[] | null defined
 *      error: code and message set in case an exception happens during the ‘textDocument/prepareCallHierarchy’ request
 */
struct CallHierarchyPrepareParams : TextDocumentPositionParams,
        WorkDoneProgressParams {
};
std::string toJsonValueStr(const CallHierarchyPrepareParams &val);

struct CallHierarchyItem
{
    std::string name;
    newlsp::Enum::SymbolKind::type_value kind;
    std::optional<std::vector<newlsp::Enum::SymbolTag::type_value>> tags;
    std::optional<std::string> detail;
    DocumentUri uri;
    Range range;
    Range selectionRange;
    std::optional<std::string> data; //unknown;
};

/** Call Hierarchy Incoming Calls
 * @brief The CallHierarchyIncomingCallsParams struct
 * Request:
 *      method: callHierarchy/incomingCalls
 *      params: CallHierarchyIncomingCallsParams defined
 * Response:
 *      result: CallHierarchyIncomingCall[] | null
 *      partial result: CallHierarchyIncomingCall[]
 *      error: code and message set in case an exception happens during the ‘callHierarchy/incomingCalls’ request
 */
struct CallHierarchyIncomingCallsParams : WorkDoneProgressParams,
        PartialResultParams {
    CallHierarchyItem item;
};
std::string toJsonValueStr(const CallHierarchyIncomingCallsParams &val);

struct CallHierarchyIncomingCall {
    CallHierarchyItem from;
    std::vector<Range> fromRanges;
};

/** Call Hierarchy Outgoing Calls
 * @brief The CallHierarchyOutgoingCallsParams struct
 * Request:
 *      method: callHierarchy/outgoingCalls
 *      params: CallHierarchyOutgoingCallsParams defined
 * Response:
 *      result: CallHierarchyOutgoingCall[] | null defined
 *      partial result: CallHierarchyOutgoingCall[]
 *      error: code and message set in case an exception happens during the ‘callHierarchy/outgoingCalls’ request
 */
struct CallHierarchyOutgoingCallsParams : WorkDoneProgressParams,
        PartialResultParams {
    CallHierarchyItem item;
};
std::string toJsonValueStr(const CallHierarchyOutgoingCallsParams &val);

struct CallHierarchyOutgoingCall {
    CallHierarchyItem to;
    std::vector<Range> fromRanges;
};

/** Prepare Type Hierarchy Request
 * @brief The TypeHierarchyPrepareParams struct
 * Request:
 *      method: ‘textDocument/prepareTypeHierarchy’
 *      params: TypeHierarchyPrepareParams defined
 * Response:
 *      result: TypeHierarchyItem[] | null defined
 *      error: code and message set in case an exception happens during the ‘textDocument/prepareTypeHierarchy’ request
 */
struct TypeHierarchyPrepareParams : TextDocumentPositionParams,
        WorkDoneProgressParams {
};
std::string toJsonValueStr(const TypeHierarchyPrepareParams &val);

struct TypeHierarchyItem {
    std::string name;
    newlsp::Enum::SymbolKind::type_value kind;
    std::optional<std::vector<newlsp::Enum::SymbolTag::type_value>> tags;
    std::optional<std::string> detail;
    DocumentUri uri;
    Range range;
    Range selectionRange;
    std::optional<std::string> data; // LSPAny
};
std::string toJsonValueStr(const TypeHierarchyItem &val);

/** Type Hierarchy Supertypes
 * @brief The TypeHierarchySupertypesParams struct
 * Request:
 *      method: ‘typeHierarchy/supertypes’
 *      params: TypeHierarchySupertypesParams defined as follows:
 * Response:
 *      result: TypeHierarchyItem[] | null
 *      partial result: TypeHierarchyItem[]
 *      error: code and message set in case an exception happens during the ‘typeHierarchy/supertypes’ request
 */
struct TypeHierarchySupertypesParams : WorkDoneProgressParams,
        PartialResultParams {
    TypeHierarchyItem item;
};
std::string toJsonValueStr(const TypeHierarchySupertypesParams &val);

/** Type Hierarchy Subtypes
 * @brief The TypeHierarchySubtypesParams struct
 * Request:
 *      method: ‘typeHierarchy/subtypes’
 *      params: TypeHierarchySubtypesParams defined as follows:
 * Response:
 *      result: TypeHierarchyItem[] | null
 *      partial result: TypeHierarchyItem[]
 *      error: code and message set in case an exception happens during the ‘typeHierarchy/subtypes’ request
 */
struct TypeHierarchySubtypesParams : WorkDoneProgressParams,
        PartialResultParams {
    TypeHierarchyItem item;
};
std::string toJsonValueStr(const TypeHierarchySubtypesParams &val);

/** Document Highlights Request
 * @brief The DocumentHighlightParams struct
 * Request:
 *      method: textDocument/documentHighlight
 *      params: DocumentHighlightParams defined
 * Response:
 *      result: DocumentHighlight[] | null defined
 *      partial result: DocumentHighlight[]
 *      error: code and message set in case an exception happens during the document highlight request.
 */
struct DocumentHighlightParams : TextDocumentPositionParams,
        WorkDoneProgressParams, PartialResultParams {
};
std::string toJsonValueStr(const DocumentHighlightParams &val);

struct DocumentHighlight {
    Range range;
    std::optional<Enum::DocumentHighlightKind::type_value> kind;
};

/** Document Link Request
 * @brief The DocumentLinkParams struct
 * Request:
 *      method: textDocument/documentLink
 *      params: DocumentLinkParams defined as follows:
 * Response:
 *      result: DocumentLink[] | null.
 *      partial result: DocumentLink[]
 *      error: code and message set in case an exception happens during the document link request.
 */
struct DocumentLinkParams : WorkDoneProgressParams,
        PartialResultParams {
    TextDocumentIdentifier textDocument;
};
std::string toJsonValueStr(const DocumentLinkParams &val);

struct DocumentLink {
    Range range;
    std::optional<DocumentUri> target;
    std::string tooltip;
    std::optional<std::string> data;
};

/** Document Link Resolve Request
 * Request:
 *      method: documentLink/resolve
 *      params: DocumentLink
 * Response:
 *      result: DocumentLink
 *      error: code and message set in case an exception happens during the document link resolve request.
 */
std::string toJsonValueStr(const DocumentLink &val);

/** Hover Request
 * @brief The HoverParams struct
 * Request:
 *      method: textDocument/hover
 *      params: HoverParams defined
 * Response:
 *      result: Hover | null defined
 *      error: code and message set in case an exception happens during the hover request.
 */
struct HoverParams : TextDocumentPositionParams,
        WorkDoneProgressParams {
};
std::string toJsonValueStr(const HoverParams &val);

struct MarkedString : std::string
{
    std::string language;
    std::string value;
    MarkedString() = default;
    MarkedString(const std::string &language, const std::string &value)
        : language(language), value(value){}
    MarkedString(const std::string &other)
        : std::string(other){
    }
};
struct Hover
{
    std::any contents; //MarkedString | MarkedString[] | MarkupContent;
    std::optional<Range> range;
};

/** Code Lens Request
 * @brief The CodeLensParams struct
 * Request:
 *      method: textDocument/codeLens
 *      params: CodeLensParams defined
 * Response:
 *      result: CodeLens[] | null defined
 *      partial result: CodeLens[]
 *      error: code and message set in case an exception happens during the code lens request.
 */
struct CodeLensParams : WorkDoneProgressParams, PartialResultParams {
    TextDocumentIdentifier textDocument;
};
std::string toJsonValueStr(const CodeLensParams &val);

struct CodeLens {
    Range range;
    std::optional<Command> command;
    std::optional<std::string> data;
};

/** Code Lens Resolve Request
 * Request:
 *      method: codeLens/resolve
 *      params: CodeLens
 * Response:
 *      result: CodeLens
 *      error: code and message set in case an exception happens during the code lens resolve request.
 */
std::string toJsonValueStr(const CodeLens &val);

/** Code Lens Refresh Request
 * Request:
 *      method: workspace/codeLens/refresh
 *      params: none
 * Response:
 *      result: void
 *      error: code and message set in case an exception happens during the ‘workspace/codeLens/refresh’ request
 */


/** Folding Range Request
 * @brief The FoldingRangeParams struct
 * Request:
 *      method: textDocument/foldingRange
 *      params: FoldingRangeParams defined
 * Response:
 *      result: FoldingRange[] | null defined
 *      partial result: FoldingRange[]
 *      error: code and message set in case an exception happens during the ‘textDocument/foldingRange’ request
 */
struct FoldingRangeParams : WorkDoneProgressParams,
        PartialResultParams {
    TextDocumentIdentifier textDocument;
};
std::string toJsonValueStr(const FoldingRangeParams &val);

struct FoldingRange {
    unsigned int startLine;
    std::optional<unsigned int> startCharacter;
    unsigned int endLine;
    std::optional<unsigned int> endCharacter;
    std::optional<newlsp::Enum::FoldingRangeKind::type_value> kind;
    std::optional<std::string> collapsedText;
};

/** Selection Range Request
 * @brief The SelectionRangeParams struct
 * Request:
 *      method: textDocument/selectionRange
 *      params: SelectionRangeParams defined
 * Response:
 *      result: SelectionRange[] | null defined
 *      partial result: SelectionRange[]
 *      error: code and message set in case an exception happens during the ‘textDocument/selectionRange’ request
 */
struct SelectionRangeParams : WorkDoneProgressParams,
        PartialResultParams {
    TextDocumentIdentifier textDocument;
    std::vector<Position> positions;
};
std::string toJsonValueStr(const std::vector<Position> &val);
std::string toJsonValueStr(const SelectionRangeParams &val);

struct SelectionRange {
    Range range;
    std::optional<std::string> parent;
};

/** Document Symbols Request
 * @brief The DocumentSymbolParams struct
 * Request:
 *      method: textDocument/documentSymbol
 *      params: DocumentSymbolParams defined
 * Response:
 *      result: DocumentSymbol[] | SymbolInformation[] | null defined
 *      partial result: DocumentSymbol[] | SymbolInformation[]. DocumentSymbol[] and SymbolInformation[] can not be mixed. That means the first chunk defines the type of all the other chunks.
 *      error: code and message set in case an exception happens during the document symbol request.
 */
struct DocumentSymbolParams : WorkDoneProgressParams,
        PartialResultParams {
    TextDocumentIdentifier textDocument;
};
std::string toJsonValueStr(const DocumentSymbolParams &val);

struct DocumentSymbol {
    QString name;
    std::optional<QString> detail;
    newlsp::Enum::SymbolKind::type_value kind;
    std::optional<std::vector<newlsp::Enum::SymbolTag::type_value>> tags;
    std::optional<bool> deprecated;
    Range range;
    Range selectionRange;
    std::optional<QList<DocumentSymbol>> children;
};
struct SymbolInformation {
    QString name;
    newlsp::Enum::SymbolKind::type_value kind;
    std::optional<std::vector<newlsp::Enum::SymbolTag::type_value>> tags;
    std::optional<bool> deprecated;
    Location location;
    std::optional<QString> containerName;
};

/** Semantic Tokens
 * @brief The SemanticTokensParams struct
 * Request:
 *      method: textDocument/semanticTokens/full
 *      params: SemanticTokensParams defined
 * Response:
 *      result: SemanticTokens | null where SemanticTokens is defined
 *      partial result: SemanticTokensPartialResult defines
 *      error: code and message set in case an exception happens during the ‘textDocument/semanticTokens/full’ request
 */
struct SemanticTokensParams : WorkDoneProgressParams,
        PartialResultParams {
    TextDocumentIdentifier textDocument;
};
std::string toJsonValueStr(const SemanticTokensParams &val);

struct SemanticTokens {
    std::optional<std::string> resultId;
    std::vector<unsigned int> data;
};
struct SemanticTokensPartialResult {
    std::vector<unsigned int> data;
};

/**
 * @brief The SemanticTokensDeltaParams struct
 * Request:
 *      method: textDocument/semanticTokens/full/delta
 *      params: SemanticTokensDeltaParams defined as follows:
 * Response:
 *      result: SemanticTokens | SemanticTokensDelta | null where SemanticTokensDelta is defined
 *      partial result: SemanticTokensDeltaPartialResult defines as follows:
 *      error: code and message set in case an exception happens during the ‘textDocument/semanticTokens/full/delta’ request
 */
struct SemanticTokensDeltaParams : WorkDoneProgressParams,
        PartialResultParams {
    TextDocumentIdentifier textDocument;
    std::string previousResultId;
};
std::string toJsonValueStr(const SemanticTokensDeltaParams &val);

struct SemanticTokensEdit {
    unsigned int start;
    unsigned int deleteCount;
    std::vector<unsigned int> data;
};
struct SemanticTokensDelta {
    std::optional<std::string> resultId;
    std::vector<SemanticTokensEdit> edits;
};
struct SemanticTokensDeltaPartialResult {
    std::vector<SemanticTokensEdit> edits;
};

/**
 * @brief The SemanticTokensRangeParams struct
 * Request:
 *      method: textDocument/semanticTokens/range
 *      params: SemanticTokensRangeParams defined as follows:
 * Response:
 *      result: SemanticTokens | null
 *      partial result: SemanticTokensPartialResult
 *      error: code and message set in case an exception happens during the ‘textDocument/semanticTokens/range’ request
 */
struct SemanticTokensRangeParams : WorkDoneProgressParams,
        PartialResultParams {
    TextDocumentIdentifier textDocument;
    Range range;
};
std::string toJsonValueStr(const SemanticTokensRangeParams &val);

/**
 * Request:
 *      method: workspace/semanticTokens/refresh
 *      params: none
 * Response:
 *      result: void
 *      error: code and message set in case an exception happens during the ‘workspace/semanticTokens/refresh’ request
 */


/** Inlay Hint Request
 * @brief The InlayHintParams struct
 * Request:
 *      method: textDocument/inlayHint
 *      params: InlayHintParams defined as follows:
 * Response:
 *      result: InlayHint[] | null defined as follows:
 *      error: code and message set in case an exception happens during the inlay hint request.
 */
struct InlayHintParams : WorkDoneProgressParams {
    TextDocumentIdentifier textDocument;
    Range range;
};
std::string toJsonValueStr(const InlayHintParams &val);

struct InlayHintLabelPart {
    std::string value;
    std::optional<std::any> tooltip; // string | MarkupContent
    std::optional<Location> location;
    std::optional<Command> command;
};
struct InlayHint {
    struct _Label : std::string, std::vector<InlayHintLabelPart>{
        _Label(const std::string &string) : std::string(string){}
        _Label(const std::vector<InlayHintLabelPart> &inlayHintLabelPart)
            : std::vector<InlayHintLabelPart>(inlayHintLabelPart){}
    };
    struct _Tooltip : std::string, MarkupContent{
        _Tooltip(const std::string &string) : std::string(string){}
        _Tooltip(const MarkupContent &content) : MarkupContent(content){}
    };
    Position position;
    _Label label;
    newlsp::Enum::InlayHintKind::type_value kind;
    std::optional<std::vector<TextEdit>> textEdits;
    std::optional<_Tooltip> tooltip;
    std::optional<bool> paddingLeft;
    std::optional<bool> paddingRight;
    std::optional<std::any> data; // LSPAny
};

/** Inlay Hint Resolve Request
 * Request:
 *      method: inlayHint/resolve
 *      params: InlayHint
 * Response:
 *      result: InlayHint
 *      error: code and message set in case an exception happens during the completion resolve request.
 */

/** Inlay Hint Refresh Request
 * Request:
 *      method: workspace/inlayHint/refresh
 *      params: none
 * Response:
 *      result: void
 *      error: code and message set in case an exception happens during the ‘workspace/inlayHint/refresh’ request
 */


/** Inline Value Request
 * @brief The InlineValueParams struct
 * Request:
 *      method: textDocument/inlineValue
 *      params: InlineValueParams defined
 * Response:
 *      result: InlineValue[] | null defined
 *      error: code and message set in case an exception happens during the inline values request.
 */
struct InlineValueContext {
    int frameId;
    Range stoppedLocation;
};
struct InlineValueParams : WorkDoneProgressParams {
    TextDocumentIdentifier textDocument;
    Range range;
    InlineValueContext context;
};
std::string toJsonValueStr(const InlineValueContext &val);
std::string toJsonValueStr(const InlineValueParams &val);

struct InlineValueText {
    Range range;
    std::string text;
};
struct InlineValueVariableLookup {
    Range range;
    std::optional<std::string> variableName;
    bool caseSensitiveLookup;
};
struct InlineValueEvaluatableExpression {
    Range range;
    std::optional<std::string> expression;
};
struct InlineValue : InlineValueText, InlineValueVariableLookup,
        InlineValueEvaluatableExpression{
    InlineValue(const Range &range, const std::string text) : InlineValueText{range, text} {}
    InlineValue(const Range &range, bool caseSensitiveLookup, const std::optional<std::string> &variableName)
        : InlineValueVariableLookup{range,variableName, caseSensitiveLookup}{}
    InlineValue(const Range &range, const std::optional<std::string> &expression)
        : InlineValueEvaluatableExpression{range, expression}{}
};

/** Inline Value Refresh Request
 *
 * Request:
 *      method: workspace/inlineValue/refresh
 *      params: none
 * Response:
 *      result: void
 *      error: code and message set in case an exception happens during the ‘workspace/inlineValue/refresh’ request
 */

/** Monikers
 * @brief The MonikerParams struct
 * Request:
 *      method: textDocument/moniker
 *      params: MonikerParams defined as follows:
 * Response:
 *      result: Moniker[] | null
 *      partial result: Moniker[]
 *      error: code and message set in case an exception happens during the ‘textDocument/moniker’ request
 */
struct MonikerParams : TextDocumentPositionParams,
        WorkDoneProgressParams, PartialResultParams {
};
std::string toJsonValueStr(const MonikerParams &val);

struct Moniker {
    std::string scheme;
    std::string identifier;
    newlsp::Enum::UniquenessLevel::type_value unique;
    std::optional<newlsp::Enum::MonikerKind::type_value> kind;
};

/** Completion Request
 * @brief The CompletionParams struct
 * Request:
 *      method: textDocument/completion
 *      params: CompletionParams defined as follows:
 * Response:
 *      result: CompletionItem[] | CompletionList | null.
 *              If a CompletionItem[] is provided it is interpreted to be complete.
 *              So it is the same as { isIncomplete: false, items }
 */
struct CompletionContext{
    newlsp::Enum::CompletionTriggerKind::type_value triggerKind;
    std::optional<std::string> triggerCharacter;
};
struct CompletionParams : TextDocumentPositionParams,
        WorkDoneProgressParams, PartialResultParams {
    std::optional<CompletionContext> context;
};
std::string toJsonValueStr(const CompletionContext &val);
std::string toJsonValueStr(const CompletionParams &val);

struct CompletionItemLabelDetails {
    std::optional<std::string> detail;
    std::optional<std::string> description;
};
struct InsertReplaceEdit {
    std::string newText;
    Range insert;
    Range replace;
};
struct DocumentationPart: std::string, MarkupContent{
    DocumentationPart(const std::string &str) : std::string(str){}
    DocumentationPart(const MarkupContent &content) : MarkupContent(content){}
};
struct TextEditPart : newlsp::TextEdit, InsertReplaceEdit{
    TextEditPart(const newlsp::TextEdit &textEdit) : newlsp::TextEdit(textEdit){}
    TextEditPart(const InsertReplaceEdit &insertReplaceEdit) : InsertReplaceEdit(insertReplaceEdit){}
};
struct CompletionItem {
    std::string label;
    std::optional<CompletionItemLabelDetails> labelDetails;
    std::optional<newlsp::Enum::CompletionItemKind::type_value> kind;
    std::optional<std::vector<newlsp::Enum::CompletionItemTag::type_value>> tags;
    std::optional<std::string> detail;
    std::optional<DocumentationPart> documentation;
    std::optional<bool> deprecated;
    std::optional<bool> preselect;
    std::optional<std::string> sortText;
    std::optional<std::string> filterText;
    std::optional<std::string> insertText;
    std::optional<newlsp::Enum::InsertTextFormat::type_value> insertTextFormat;
    std::optional<newlsp::Enum::InsertTextMode::type_value> insertTextMode;
    std::optional<TextEditPart> textEdit;
    std::optional<std::string> textEditText;
    std::optional<std::vector<TextEdit>> additionalTextEdits;
    std::optional<std::vector<std::string>> commitCharacters;
    std::optional<Command> command;
    std::optional<std::any> data; // LSPAny;
};
struct CompletionList
{
    bool isIncomplete;
    struct ItemDefaults {
        struct EditRange : Range {
            Range insert;
            Range replace;
            EditRange(const Range &range) : Range(range){}
            EditRange(const Range &insert, const Range &replace) : insert(insert), replace(replace){}
        };
        std::optional<std::vector<std::string>> commitCharacters;
        std::optional<EditRange> editRange;
        std::optional<newlsp::Enum::InsertTextFormat::type_value> insertTextFormat;
        std::optional<newlsp::Enum::InsertTextMode::type_value> insertTextMode;
        std::optional<std::any> data;
    };
    std::optional<ItemDefaults> itemDefaults;
    std::vector<CompletionItem> items;
};

/** Completion Item Resolve Request
 * The request is sent from the client to the server to resolve additional information for a given completion item.
 * Request:
 *      method: completionItem/resolve
 *      params: CompletionItem
 * Response:
 *      result: CompletionItem
 *      error: code and message set in case an exception happens during the completion resolve request.
 */

/** PublishDiagnostics Notification
 * caller is server
 * Notification:
 *      method: textDocument/publishDiagnostics
 *      params: PublishDiagnosticsParams defined
 */
struct PublishDiagnosticsParams {
    DocumentUri uri;
    std::optional<int> version;
    std::vector<Diagnostic> diagnostics;
};

/** Document Diagnostics Request
 * @brief The DocumentDiagnosticParams struct
 * Request:
 *      method: ‘textDocument/diagnostic’.
 *      params: DocumentDiagnosticParams defined
 * Response:
 *      result: DocumentDiagnosticReport defined
 *      partial result: The first literal send need to be a DocumentDiagnosticReport followed by
 *                      n DocumentDiagnosticReportPartialResult literals defined
 *      error: code and message set in case an exception happens during the diagnostic request.
 *             A server is also allowed to return an error with code ServerCancelled indicating
 *             that the server can’t compute the result right now.
 *             A server can return a DiagnosticServerCancellationData data to indicate whether the
 *             client should re-trigger the request.
 *             If no data is provided it defaults to { retriggerRequest: true }:
 */
struct DocumentDiagnosticParams : WorkDoneProgressParams,
        PartialResultParams {
    TextDocumentIdentifier textDocument;
    std::optional<std::string> identifier;
    std::optional<std::string> previousResultId;
};
std::string toJsonValueStr(const DocumentDiagnosticParams &val);

enum_def(DocumentDiagnosticReportKind, std::string) {
    enum_exp Full = "Full";
    enum_exp Unchanged = "unchanged";
};
struct FullDocumentDiagnosticReport {
    DocumentDiagnosticReportKind::type_value kind = DocumentDiagnosticReportKind::get()->Full;
    std::optional<std::string> resultId;
    std::vector<Diagnostic> items;
};
struct UnchangedDocumentDiagnosticReport {
    DocumentDiagnosticReportKind::type_value kind = DocumentDiagnosticReportKind::get()->Unchanged;
    std::string resultId;
};
struct RelatedDocuments_Value : FullDocumentDiagnosticReport, UnchangedDocumentDiagnosticReport{
    RelatedDocuments_Value(const FullDocumentDiagnosticReport &full)
        : FullDocumentDiagnosticReport(full){}
    RelatedDocuments_Value(const UnchangedDocumentDiagnosticReport &unchanged)
        : UnchangedDocumentDiagnosticReport(unchanged){}
};
typedef std::string RelatedDocuments_Key;
struct RelatedDocuments : std::pair<RelatedDocuments_Value, RelatedDocuments_Key> {};
struct RelatedFullDocumentDiagnosticReport : FullDocumentDiagnosticReport {
    std::optional<RelatedDocuments> relatedDocuments;
};
struct RelatedUnchangedDocumentDiagnosticReport : UnchangedDocumentDiagnosticReport {
    struct _Value : FullDocumentDiagnosticReport, UnchangedDocumentDiagnosticReport{
        _Value(const FullDocumentDiagnosticReport &full)
            : FullDocumentDiagnosticReport(full){}
        _Value(const UnchangedDocumentDiagnosticReport &unchanged)
            : UnchangedDocumentDiagnosticReport(unchanged){}
    };
    typedef std::string _Key;
    typedef std::optional<std::pair<_Key, _Value>> _RelatedDocuments;
    std::optional<_RelatedDocuments> relatedDocuments;
};
struct DocumentDiagnosticReport : RelatedFullDocumentDiagnosticReport, RelatedUnchangedDocumentDiagnosticReport {
    DocumentDiagnosticReport(const RelatedFullDocumentDiagnosticReport &full)
        : RelatedFullDocumentDiagnosticReport(full){}
    DocumentDiagnosticReport(const RelatedUnchangedDocumentDiagnosticReport &unchanged)
        : RelatedUnchangedDocumentDiagnosticReport(unchanged){}
};
struct DocumentDiagnosticReportPartialResult {
    RelatedDocuments relatedDocuments;
};
struct DiagnosticServerCancellationData {
    std::optional<bool> retriggerRequest;
};

/** Workspace Diagnostics Request
 * @brief The WorkspaceDiagnosticParams struct
 * Request:
 *      method: ‘workspace/diagnostic’.
 *      params: WorkspaceDiagnosticParams defined as follows:
 * Response:
 *      result: WorkspaceDiagnosticReport defined as follows:
 *      error: code and message set in case an exception happens during the diagnostic request.
 *             A server is also allowed to return and error with
 *             code ServerCancelled indicating that the server can’t compute the result right now.
 *             A server can return a DiagnosticServerCancellationData data to indicate whether
 *             the client should re-trigger the request.
 *             If no data is provided it defaults to { retriggerRequest: true }
 */
struct PreviousResultId {
    DocumentUri uri;
    std::string value;
};
struct WorkspaceDiagnosticParams : WorkDoneProgressParams,
        PartialResultParams {
    std::optional<std::string> identifier;
    std::vector<PreviousResultId> previousResultIds;
};
std::string toJsonValueStr(const PreviousResultId &val);
std::string toJsonValueStr(const WorkspaceDiagnosticParams &val);

struct WorkspaceFullDocumentDiagnosticReport : FullDocumentDiagnosticReport {
    DocumentUri uri;
    std::optional<int> version; // null;
};
struct WorkspaceUnchangedDocumentDiagnosticReport : UnchangedDocumentDiagnosticReport {
    DocumentUri uri;
    std::optional<int> version; // null;
};
struct WorkspaceDocumentDiagnosticReport : WorkspaceFullDocumentDiagnosticReport,
        WorkspaceUnchangedDocumentDiagnosticReport{
    WorkspaceDocumentDiagnosticReport(const WorkspaceFullDocumentDiagnosticReport &full)
        : WorkspaceFullDocumentDiagnosticReport(full){}
    WorkspaceDocumentDiagnosticReport(const WorkspaceUnchangedDocumentDiagnosticReport &unchanged)
        : WorkspaceUnchangedDocumentDiagnosticReport(unchanged){}
};
struct WorkspaceDiagnosticReport {
    std::vector<WorkspaceDocumentDiagnosticReport> items;
};

/** Diagnostics Refresh
 * Request:
 *      method: workspace/diagnostic/refresh
 *      params: none
 * Response:
 *      result: void
 *      error: code and message set in case an exception happens during the
 *             ‘workspace/diagnostic/refresh’ request
 */


/** Signature Help Request
 * @brief The SignatureHelpParams struct
 * Request:
 *      method: textDocument/signatureHelp
 *      params: SignatureHelpParams defined
 * Response:
 *      result: SignatureHelp | null defined
 *      error: code and message set in case an exception happens during the signature help request.
 */
struct SignatureHelpParams : TextDocumentPositionParams,
        WorkDoneProgressParams {
    struct SignatureHelpContext {
        struct SignatureHelp {
            struct SignatureInformation {
                struct ParameterInformation {
                    std::string label; // string | [uinteger, uinteger]; ???
                    std::optional<DocumentationPart> documentation;
                };
                std::string label;
                std::optional<DocumentationPart> documentation;
                std::optional<std::vector<ParameterInformation>> parameters;
                std::optional<unsigned int> activeParameter;
            };
            std::vector<SignatureInformation> signatures;
            std::optional<unsigned int> activeSignature;
            std::optional<unsigned int> activeParameter;
        };
        Enum::SignatureHelpTriggerKind::type_value triggerKind;
        std::optional<std::string> triggerCharacter;
        bool isRetrigger;
        std::optional<SignatureHelp> activeSignatureHelp;
    };
    std::optional<SignatureHelpContext> context;
};
std::string toJsonValueStr(const SignatureHelpParams::SignatureHelpContext::SignatureHelp::SignatureInformation::ParameterInformation &val);
std::string toJsonValueStr(const SignatureHelpParams::SignatureHelpContext::SignatureHelp::SignatureInformation &val);
std::string toJsonValueStr(const SignatureHelpParams::SignatureHelpContext::SignatureHelp &val);
std::string toJsonValueStr(const SignatureHelpParams::SignatureHelpContext &val);
std::string toJsonValueStr(const SignatureHelpParams &val);

/** Code Action Request
 * @brief The CodeActionParams struct
 * Request:
 *      method: textDocument/codeAction
 *      params: CodeActionParams defined
 * Response:
 *      result: (Command | CodeAction)[] | null where CodeAction is defined
 *      partial result: (Command | CodeAction)[]
 *      error: code and message set in case an exception happens during the code action request.
 */
struct CodeActionContext {
    std::vector<Diagnostic> diagnostics;
    std::optional<std::vector<Enum::CodeActionKind::type_value>> only;
    std::optional<Enum::CodeActionTriggerKind::type_value> triggerKind;
};
struct CodeActionParams : WorkDoneProgressParams, PartialResultParams {
    TextDocumentIdentifier textDocument;
    Range range;
    CodeActionContext context;
};
std::string toJsonValueStr(const CodeActionContext &val);
std::string toJsonValueStr(const CodeActionParams &val);

/** Code Action Resolve Request
 * Request:
 *      method: codeAction/resolve
 *      params: CodeAction
 * Response:
 *      result: CodeAction
 *      error: code and message set in case an exception happens during the completion resolve request.
 */

/** Document Color Request
 * @brief The DocumentColorParams struct
 * Request:
 *      method: textDocument/documentColor
 *      params: DocumentColorParams defined
 * Response:
 *      result: ColorInformation[] defined
 *      partial result: ColorInformation[]
 *      error: code and message set in case an exception happens during the ‘textDocument/documentColor’ request
 */
struct DocumentColorParams : WorkDoneProgressParams, PartialResultParams {
    TextDocumentIdentifier textDocument;
};
std::string toJsonValueStr(const DocumentColorParams &val);

struct Color {
    float red;
    float green;
    float blue;
    float alpha;
};
std::string toJsonValueStr(const Color &val);

struct ColorInformation {
    Range range;
    Color color;
};

/** Color Presentation Request
 * @brief The ColorPresentationParams struct
 * Request:
 *      method: textDocument/colorPresentation
 *      params: ColorPresentationParams defined
 * Response:
 *      result: ColorPresentation[] defined
 *      partial result: ColorPresentation[]
 *      error: code and message set in case an exception happens during the ‘textDocument/colorPresentation’ request
 */
struct ColorPresentationParams : WorkDoneProgressParams, PartialResultParams {
    TextDocumentIdentifier textDocument;
    Color color;
    Range range;
};
std::string toJsonValueStr(const ColorPresentationParams &val);

struct ColorPresentation {
    std::string label;
    std::optional<TextEdit> textEdit;
    std::optional<std::vector<TextEdit>> additionalTextEdits;
};

/** Document Formatting Request
 * @brief The DocumentFormattingParams struct
 * Request:
 *      method: textDocument/formatting
 *      params: DocumentFormattingParams defined as follows
 * Response:
 *      result: TextEdit[] | null describing the modification to the document to be formatted.
 *      error: code and message set in case an exception happens during the formatting request.
 */
struct FormattingOptions : std::vector<std::pair<std::string, std::any>> {
    unsigned int tabSize;
    bool insertSpaces;
    std::optional<bool> trimTrailingWhitespace;
    std::optional<bool> insertFinalNewline;
    std::optional<bool> trimFinalNewlines;
};
struct DocumentFormattingParams : WorkDoneProgressParams {
    TextDocumentIdentifier textDocument;
    FormattingOptions options;
};
std::string toJsonValueStr(const FormattingOptions &val);
std::string toJsonValueStr(const DocumentFormattingParams &val);

/** Document Range Formatting Request
 * @brief The DocumentRangeFormattingParams struct
 * Request:
 *      method: textDocument/rangeFormatting,
 *      params: DocumentRangeFormattingParams defined
 * Response:
 *      result: TextEdit[] | null describing the modification to the document to be formatted.
 *      error: code and message set in case an exception happens during the range formatting request.
 */
struct DocumentRangeFormattingParams : WorkDoneProgressParams {
    TextDocumentIdentifier textDocument;
    Range range;
    FormattingOptions options;
};
std::string toJsonValueStr(const DocumentRangeFormattingParams &val);

/** Document on Type Formatting Request
 * @brief The DocumentOnTypeFormattingParams struct
 * Request:
 *      method: textDocument/onTypeFormatting
 *      params: DocumentOnTypeFormattingParams defined as follows:
 * Response:
 *      result: TextEdit[] | null describing the modification to the document.
 *      error: code and message set in case an exception happens during the range formatting request.
 */
struct DocumentOnTypeFormattingParams {
    TextDocumentIdentifier textDocument;
    Position position;
    std::string ch;
    FormattingOptions options;
};
std::string toJsonValueStr(const DocumentOnTypeFormattingParams &val);

/** Rename Request
 * @brief The RenameParams struct
 * Request:
 *      method: textDocument/rename
 *      params: RenameParams defined as follows
 * Response:
 *      result: WorkspaceEdit | null describing the modification to the workspace.
 *              null should be treated the same was as WorkspaceEdit with no changes (no change was required).
 *      error: code and message set in case when rename could not be performed for any reason.
 *             Examples include: there is nothing at given position to rename (like a space),
 *             given symbol does not support renaming by the server or the code is invalid (e.g. does not compile).
 */
struct RenameParams : TextDocumentPositionParams,
        WorkDoneProgressParams {
    std::string newName;
};
std::string toJsonValueStr(const RenameParams &val);

/** Prepare Rename Request
 * @brief The PrepareRenameParams struct
 * Request:
 *      method: textDocument/prepareRename
 *      params: PrepareRenameParams defined
 * Response:
 *      result: Range | { range: Range, placeholder: string } | { defaultBehavior: boolean } | null
 *              describing a Range of the string to rename and optionally a placeholder text of
 *              the string content to be renamed. If { defaultBehavior: boolean } is returned (since 3.16)
 *              the rename position is valid and the client should use its default behavior to compute the rename range.
 *              If null is returned then it is deemed that a ‘textDocument/rename’ request is not valid at the given position.
 *      error: code and message set in case the element can’t be renamed.
 *             Clients should show the information in their user interface.
 */
struct PrepareRenameParams : TextDocumentPositionParams
        , WorkDoneProgressParams {
};
std::string toJsonValueStr(const PrepareRenameParams &val);

/** Linked Editing Range
 * @brief The LinkedEditingRangeParams struct
 * Request:
 *      method: textDocument/linkedEditingRange
 *      params: LinkedEditingRangeParams defined
 * Response:
 *      result: LinkedEditingRanges | null defined
 *      error: code and message set in case an exception happens during the ‘textDocument/linkedEditingRange’ request
 */
struct LinkedEditingRangeParams : TextDocumentPositionParams
        , WorkDoneProgressParams {
};
std::string toJsonValueStr(const LinkedEditingRangeParams &val);

struct LinkedEditingRanges {
    std::vector<Range> ranges;
    std::optional<std::string> wordPattern;
};

} // namespace newlsp
#endif // LANGUAGEFEATURES_H
