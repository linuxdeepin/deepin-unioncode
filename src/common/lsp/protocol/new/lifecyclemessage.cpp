// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lifecyclemessage.h"

namespace newlsp {
using namespace json;
std::string toJsonValueStr(const ResolveSupport &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"properties", val.properties});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CompletionClientCapabilities::CompletionList &val)
{
    std::string ret;
    if (val.itemDefaults)
        ret = json::addValue(ret, json::KV{"itemDefaults", val.itemDefaults});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CompletionClientCapabilities::CompletionItemKind &val)
{
    std::string ret;
    if (val.valueSet)
        ret = json::addValue(ret, json::KV{"valueSet", val.valueSet});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CompletionClientCapabilities::CompletionItem::TagSupport &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"valueSet", val.valueSet});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CompletionClientCapabilities::CompletionItem &val)
{
    std::string ret;
    if (val.snippetSupport)
        ret = json::addValue(ret, json::KV{"snippetSupport", val.snippetSupport});

    if (val.commitCharactersSupport)
        ret = json::addValue(ret, json::KV{"commitCharactersSupport", val.commitCharactersSupport});

    if (val.documentationFormat)
        ret = json::addValue(ret, json::KV{"documentationFormat", val.documentationFormat});

    if (val.deprecatedSupport)
        ret = json::addValue(ret, json::KV{"deprecatedSupport", val.deprecatedSupport});

    if (val.preselectSupport)
        ret = json::addValue(ret, json::KV{"preselectSupport", val.preselectSupport});

    if (val.tagSupport)
        ret = json::addValue(ret, json::KV{"tagSupport", val.tagSupport});

    if (val.insertReplaceSupport)
        ret = json::addValue(ret, json::KV{"insertReplaceSupport", val.insertReplaceSupport});

    if (val.resolveSupport)
        ret = json::addValue(ret, json::KV{"resolveSupport", val.resolveSupport});

    if (val.insertTextModeSupport)
        ret = json::addValue(ret, json::KV{"insertTextModeSupport", val.insertTextModeSupport});

    if (val.labelDetailsSupport)
        ret = json::addValue(ret, json::KV{"labelDetailsSupport", val.labelDetailsSupport});

    return json::addScope(ret);
}

std::string toJsonValueStr(const CodeActionLiteralSupport &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"codeActionKind", val.codeActionKind});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SignatureHelpClientCapabilities::SignatureInformation::ParameterInformation &val)
{
    std::string ret;
    if (val.labelOffsetSupport)
        ret = json::addValue(ret, json::KV{"labelOffsetSupport", val.labelOffsetSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SignatureHelpClientCapabilities::SignatureInformation &val)
{
    std::string ret;
    if (val.documentationFormat)
        ret = json::addValue(ret, json::KV{"documentationFormat", val.documentationFormat});

    if (val.parameterInformation)
        ret = json::addValue(ret, json::KV{"parameterInformation", val.parameterInformation});

    if (val.activeParameterSupport)
        ret = json::addValue(ret, json::KV{"activeParameterSupport", val.activeParameterSupport});

    return json::addScope(ret);
}

std::string toJsonValueStr(const CodeActionKind &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"valueSet", val.valueSet});
    return json::addScope(ret);
}

std::string toJsonValueStr(const ChangeAnotationSupport &val)
{
    std::string ret;
    if (val.groupsOnLabel)
        ret = json::addValue(ret, json::KV{"groupsOnLabel", val.groupsOnLabel});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SymbolKind &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"valueSet", val.valueSet});
    return json::addScope(ret);
}

std::string toJsonValueStr(const WorkspaceEditClientCapabilities &val)
{
    std::string ret;
    if (val.documentChanges)
        ret = json::addValue(ret, json::KV{"documentChanges", val.documentChanges});
    if (val.resourceOperations)
        ret = json::addValue(ret, json::KV{"resourceOperations", val.resourceOperations});
    if (val.failureHandling)
        ret = json::addValue(ret, json::KV{"failureHandling", val.failureHandling});
    if (val.normalizesLineEndings)
        ret = json::addValue(ret, json::KV{"normalizesLineEndings", val.normalizesLineEndings});
    if (val.changeAnnotationSupport)
        ret = json::addValue(ret, json::KV{"changeAnnotationSupport", val.changeAnnotationSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DidChangeConfigurationClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DidChangeWatchedFilesClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.relativePatternSupport)
        ret = json::addValue(ret, json::KV{"relativePatternSupport", val.relativePatternSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const ExecuteCommandClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const WorkspaceSymbolClientCapabilities::TagSupport &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"valueSet", val.valueSet});
    return json::addScope(ret);
}

std::string toJsonValueStr(const WorkspaceSymbolClientCapabilities &val){
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.symbolKind)
        ret = json::addValue(ret, json::KV{"symbolKind", val.symbolKind});
    if (val.tagSupport)
        ret = json::addValue(ret, json::KV{"tagSupport", val.tagSupport});
    if (val.resolveSupport)
        ret = json::addValue(ret, json::KV{"resolveSupport", val.resolveSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SemanticTokensWorkspaceClientCapabilities &val)
{
    std::string ret;
    if (val.refreshSupport)
        ret = json::addValue(ret, json::KV{"refreshSupport", val.refreshSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CodeLensWorkspaceClientCapabilities &val)
{
    std::string ret;
    if (val.refreshSupport)
        ret = json::addValue(ret, json::KV{"refreshSupport", val.refreshSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const InlineValueWorkspaceClientCapabilities &val)
{
    std::string ret;
    if (val.refreshSupport)
        ret = json::addValue(ret, json::KV{"refreshSupport", val.refreshSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const InlayHintWorkspaceClientCapabilities &val)
{
    std::string ret;
    if (val.refreshSupport)
        ret = json::addValue(ret, json::KV{"refreshSupport", val.refreshSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DiagnosticWorkspaceClientCapabilities &val)
{
    std::string ret;
    if (val.refreshSupport)
        ret = json::addValue(ret, json::KV{"refreshSupport", val.refreshSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TextDocumentSyncClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.willSave)
        ret = json::addValue(ret, json::KV{"willSave", val.willSave});
    if (val.willSaveWaitUntil)
        ret = json::addValue(ret, json::KV{"willSaveWaitUntil", val.willSaveWaitUntil});
    if (val.didSave)
        ret = json::addValue(ret, json::KV{"didSave", val.didSave});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CompletionClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.completionItem)
        ret = json::addValue(ret, json::KV{"completionItem", val.completionItem});
    if (val.completionItemKind)
        ret = json::addValue(ret, json::KV{"completionItemKind", val.completionItemKind});
    if (val.contextSupport)
        ret = json::addValue(ret, json::KV{"contextSupport", val.contextSupport});
    if (val.insertTextMode)
        ret = json::addValue(ret, json::KV{"insertTextMode", val.insertTextMode});
    if (val.itemDefaults)
        ret = json::addValue(ret, json::KV{"itemDefaults", val.itemDefaults});
    return json::addScope(ret);
}

std::string toJsonValueStr(const HoverClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.contentFormat)
        ret = json::addValue(ret, json::KV{"contentFormat", val.contentFormat});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SignatureHelpClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.signatureInformation)
        ret = json::addValue(ret, json::KV{"signatureInformation", val.signatureInformation});
    if (val.contextSupport)
        ret = json::addValue(ret, json::KV{"contextSupport", val.contextSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DeclarationClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.linkSupport)
        ret = json::addValue(ret, json::KV{"linkSupport", val.linkSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DefinitionClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.linkSupport)
        ret = json::addValue(ret, json::KV{"linkSupport", val.linkSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TypeDefinitionClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.linkSupport)
        ret = json::addValue(ret, json::KV{"linkSupport", val.linkSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const ImplementationClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.linkSupport)
        ret = json::addValue(ret, json::KV{"linkSupport", val.linkSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const ReferenceClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentHighlightClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentSymbolClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.symbolKind)
        ret = json::addValue(ret, json::KV{"symbolKind", val.symbolKind});
    if (val.hierarchicalDocumentSymbolSupport)
        ret = json::addValue(ret, json::KV{"hierarchicalDocumentSymbolSupport",
                                           val.hierarchicalDocumentSymbolSupport});
    if (val.tagSupport)
        ret = json::addValue(ret, json::KV{"tagSupport", val.tagSupport});
    if (val.labelSupport)
        ret = json::addValue(ret, json::KV{"labelSupport", val.labelSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CodeActionClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.codeActionLiteralSupport)
        ret = json::addValue(ret, json::KV{"codeActionLiteralSupport", val.codeActionLiteralSupport});
    if (val.isPreferredSupport)
        ret = json::addValue(ret, json::KV{"isPreferredSupport", val.isPreferredSupport});
    if (val.disabledSupport)
        ret = json::addValue(ret, json::KV{"disabledSupport", val.disabledSupport});
    if (val.dataSupport)
        ret = json::addValue(ret, json::KV{"dataSupport", val.dataSupport});
    if (val.resolveSupport)
        ret = json::addValue(ret, json::KV{"resolveSupport", val.resolveSupport});
    if (val.honorsChangeAnnotations)
        ret = json::addValue(ret, json::KV{"honorsChangeAnnotations", val.honorsChangeAnnotations});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CodeLensClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentLinkClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.tooltipSupport)
        ret = json::addValue(ret, json::KV{"tooltipSupport", val.tooltipSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentColorClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentFormattingClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentRangeFormattingClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentOnTypeFormattingClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const RenameClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.prepareSupport)
        ret = json::addValue(ret, json::KV{"prepareSupport", val.prepareSupport});
    if (val.prepareSupportDefaultBehavior)
        ret = json::addValue(ret, json::KV{"prepareSupportDefaultBehavior", val.prepareSupportDefaultBehavior});
    if (val.honorsChangeAnnotations)
        ret = json::addValue(ret, json::KV{"honorsChangeAnnotations", val.honorsChangeAnnotations});
    return json::addScope(ret);
}

std::string toJsonValueStr(const PublishDiagnosticsClientCapabilities &val)
{
    std::string ret;
    if (val.relatedInformation)
        ret = json::addValue(ret, json::KV{"relatedInformation", val.relatedInformation});
    if (val.tagSupport)
        ret = json::addValue(ret, json::KV{"tagSupport", val.tagSupport});
    if (val.versionSupport)
        ret = json::addValue(ret, json::KV{"versionSupport", val.versionSupport});
    if (val.codeDescriptionSupport)
        ret = json::addValue(ret, json::KV{"codeDescriptionSupport", val.codeDescriptionSupport});
    if (val.dataSupport)
        ret = json::addValue(ret, json::KV{"dataSupport", val.dataSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const FoldingRangeClientCapabilities::FoldingRangeKind &val)
{
    std::string ret;
    if (val.valueSet)
        ret = json::addValue(ret, json::KV{"valueSet", val.valueSet});
    return json::addScope(ret);
}

std::string toJsonValueStr(const FoldingRangeClientCapabilities::FoldingRange &val)
{
    std::string ret;
    if (val.collapsedText)
        ret = json::addValue(ret, json::KV{"collapsedText", val.collapsedText});
    return json::addScope(ret);
}

std::string toJsonValueStr(const FoldingRangeClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.rangeLimit)
        ret = json::addValue(ret, json::KV{"rangeLimit", val.rangeLimit});
    if (val.lineFoldingOnly)
        ret = json::addValue(ret, json::KV{"lineFoldingOnly", val.lineFoldingOnly});
    if (val.foldingRangeKind)
        ret = json::addValue(ret, json::KV{"foldingRangeKind", val.foldingRangeKind});
    if (val.foldingRange)
        ret = json::addValue(ret, json::KV{"foldingRange", val.foldingRange});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SelectionRangeClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const LinkedEditingRangeClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CallHierarchyClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SemanticTokensClientCapabilities::Requests::Full &val)
{
    std::string ret;
    if (val.delta) {
        ret = json::addValue(ret, json::KV{"delta", val.delta});
    }
    return json::addScope(ret);
}

std::string toJsonValueStr(const SemanticTokensClientCapabilities::Requests &val)
{
    std::string ret;
    if (val.range) {
        if (any_contrast<bool>(val.range.value())) {
            ret = json::addValue(ret, json::KV{"range", std::any_cast<bool>(val.range.value())});
        } else {
            ret = json::addValue(ret, json::KV{"range", std::string("{}")});
        }
    }
    if (val.full) {
        ret = json::addValue(ret, json::KV{"full", val.full});
    }
    return json::addScope(ret);
}

std::string toJsonValueStr(const SemanticTokensClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    ret = json::addValue(ret, json::KV{"requests", val.requests});
    ret = json::addValue(ret, json::KV{"tokenTypes", val.tokenTypes});
    ret = json::addValue(ret, json::KV{"tokenModifiers", val.tokenModifiers});
    ret = json::addValue(ret, json::KV{"formats", val.formats});
    if (val.overlappingTokenSupport)
        ret = json::addValue(ret, json::KV{"overlappingTokenSupport", val.overlappingTokenSupport});
    if (val.multilineTokenSupport)
        ret = json::addValue(ret, json::KV{"multilineTokenSupport", val.multilineTokenSupport});
    if (val.serverCancelSupport)
        ret = json::addValue(ret, json::KV{"serverCancelSupport", val.multilineTokenSupport});
    if (val.augmentsSyntaxTokens)
        ret = json::addValue(ret, json::KV{"augmentsSyntaxTokens", val.multilineTokenSupport});
    return json::addScope(ret);
}



std::string toJsonValueStr(const MonikerClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TypeHierarchyClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const InlineValueClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const InlayHintClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.resolveSupport)
        ret = json::addValue(ret, json::KV{"resolveSupport", val.resolveSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DiagnosticClientCapabilities &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.relatedDocumentSupport)
        ret = json::addValue(ret, json::KV{"relatedDocumentSupport", val.relatedDocumentSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TextDocumentClientCapabilities &val) {
    std::string ret;
    if (val.synchronization)
        ret = json::addValue(ret, json::KV{"synchronization", val.synchronization});
    if (val.completion)
        ret = json::addValue(ret, json::KV{"completion", val.completion});
    if (val.hover)
        ret = json::addValue(ret, json::KV{"hover", val.hover});
    if (val.signatureHelp)
        ret = json::addValue(ret, json::KV{"signatureHelp", val.signatureHelp});
    if (val.declaration)
        ret = json::addValue(ret, json::KV{"declaration", val.declaration});
    if (val.definition)
        ret = json::addValue(ret, json::KV{"definition", val.definition});
    if (val.typeDefinition)
        ret = json::addValue(ret, json::KV{"typeDefinition", val.typeDefinition});
    if (val.implementation)
        ret = json::addValue(ret, json::KV{"implementation", val.implementation});
    if (val.references)
        ret = json::addValue(ret, json::KV{"references", val.references});
    if (val.documentHighlight)
        ret = json::addValue(ret, json::KV{"documentHighlight", val.documentHighlight});
    if (val.documentSymbol)
        ret = json::addValue(ret, json::KV{"documentSymbol", val.documentSymbol});
    if (val.codeAction)
        ret = json::addValue(ret, json::KV{"codeAction", val.codeAction});
    if (val.codeLens)
        ret = json::addValue(ret, json::KV{"codeLens", val.codeLens});
    if (val.documentLink)
        ret = json::addValue(ret, json::KV{"documentLink", val.documentLink});
    if (val.colorProvider)
        ret = json::addValue(ret, json::KV{"colorProvider", val.colorProvider});
    if (val.formatting)
        ret = json::addValue(ret, json::KV{"formatting", val.formatting});
    if (val.rangeFormatting)
        ret = json::addValue(ret, json::KV{"rangeFormatting", val.formatting});
    if (val.onTypeFormatting)
        ret = json::addValue(ret, json::KV{"onTypeFormatting", val.onTypeFormatting});
    if (val.rename)
        ret = json::addValue(ret, json::KV{"rename", val.rename});
    if (val.publishDiagnostics)
        ret = json::addValue(ret, json::KV{"publishDiagnostics", val.publishDiagnostics});
    if (val.foldingRange)
        ret = json::addValue(ret, json::KV{"foldingRange", val.foldingRange});
    if (val.selectionRange)
        ret = json::addValue(ret, json::KV{"selectionRange", val.selectionRange});
    if (val.linkedEditingRange)
        ret = json::addValue(ret, json::KV{"linkedEditingRange", val.linkedEditingRange});
    if (val.callHierarchy)
        ret = json::addValue(ret, json::KV{"callHierarchy", val.callHierarchy});
    if (val.semanticTokens)
        ret = json::addValue(ret, json::KV{"semanticTokens", val.semanticTokens});
    if (val.moniker)
        ret = json::addValue(ret, json::KV{"moniker", val.moniker});
    if (val.typeHierarchy)
        ret = json::addValue(ret, json::KV{"typeHierarchy", val.typeHierarchy});
    if (val.inlineValue)
        ret = json::addValue(ret, json::KV{"inlineValue", val.inlineValue});
    if (val.inlayHint)
        ret = json::addValue(ret, json::KV{"inlayHint", val.inlayHint});
    if (val.diagnostic)
        ret = json::addValue(ret, json::KV{"diagnostic", val.diagnostic});
    return json::addScope(ret);
}

std::string toJsonValueStr(const FileOperations &val)
{
    std::string ret;
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.didCreate)
        ret = json::addValue(ret, json::KV{"didCreate", val.didCreate});
    if (val.willCreate)
        ret = json::addValue(ret, json::KV{"willCreate", val.willCreate});
    if (val.didRename)
        ret = json::addValue(ret, json::KV{"didRename", val.didRename});
    if (val.willRename)
        ret = json::addValue(ret, json::KV{"willRename", val.willRename});
    if (val.didDelete)
        ret = json::addValue(ret, json::KV{"didDelete", val.didDelete});
    if (val.willDelete)
        ret = json::addValue(ret, json::KV{"willDelete", val.willDelete});
    return json::addScope(ret);
}

std::string toJsonValueStr(const Workspace &val)
{
    std::string ret;
    if (val.applyEdit)
        ret = json::addValue(ret, json::KV{"applyEdit", val.applyEdit});
    if (val.workspaceEdit)
        ret = json::addValue(ret, json::KV{"workspaceEdit", val.workspaceEdit});
    if (val.didChangeConfiguration)
        ret = json::addValue(ret, json::KV{"didChangeConfiguration", val.didChangeConfiguration});
    if (val.didChangeWatchedFiles)
        ret = json::addValue(ret, json::KV{"didChangeWatchedFiles", val.didChangeWatchedFiles});
    if (val.symbol)
        ret = json::addValue(ret, json::KV{"symbol", val.symbol});
    if (val.executeCommand)
        ret = json::addValue(ret, json::KV{"executeCommand", val.executeCommand});
    if (val.workspaceFolders)
        ret = json::addValue(ret, json::KV{"workspaceFolders", val.workspaceFolders});
    if (val.configuration)
        ret = json::addValue(ret, json::KV{"configuration", val.configuration});
    if (val.semanticTokens)
        ret = json::addValue(ret, json::KV{"semanticTokens", val.semanticTokens});
    if (val.codeLens)
        ret = json::addValue(ret, json::KV{"codeLens", val.codeLens});
    if (val.fileOperations)
        ret = json::addValue(ret, json::KV{"fileOperations", val.fileOperations});
    if (val.inlineValue)
        ret = json::addValue(ret, json::KV{"inlineValue", val.inlineValue});
    if (val.inlayHint)
        ret = json::addValue(ret, json::KV{"inlayHint", val.inlayHint});
    if (val.diagnostics)
        ret = json::addValue(ret, json::KV{"diagnostics", val.diagnostics});
    return json::addScope(ret);
}

std::string toJsonValueStr(const ClientInfo &val)
{
    std::string result{};
    result = json::addValue(result, json::KV{"name", val.name});
    if (val.version)
        result = json::addValue(result, json::KV{"version", val.version});
    return json::addScope(result);
}

std::string toJsonValueStr(const NotebookDocumentSyncClientCapabilities &val)
{
    std::string ret = "";
    if (val.dynamicRegistration)
        ret = json::addValue(ret, json::KV{"dynamicRegistration", val.dynamicRegistration});
    if (val.executionSummarySupport)
        ret = json::addValue(ret, json::KV{"executionSummarySupport", val.executionSummarySupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const NotebookDocumentClientCapabilities &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"synchronization", val.synchronization});
    return json::addScope(ret);
}

std::string toJsonValueStr(const MessageActionItem &val)
{
    std::string ret;
    if (val.additionalPropertiesSupport)
        ret = json::addValue(ret, json::KV{"additionalPropertiesSupport", val.additionalPropertiesSupport});
    return json::addScope(ret);
}

std::string toJsonValueStr(const ShowMessageRequestClientCapabilities &val)
{
    std::string ret;
    if (val.messageActionItem)
        ret = json::addValue(ret, json::KV{"messageActionItem", val.messageActionItem});
    return json::addScope(ret);
}

std::string toJsonValueStr(const ShowDocumentClientCapabilities &val) {
    std::string ret;
    ret = json::addValue(ret, json::KV{"support", val.support});
    return json::addScope(ret);
}

std::string toJsonValueStr(const Window &val)
{
    std::string ret;
    if (val.workDoneProgress)
        ret = json::addValue(ret, json::KV{"workDoneProgress", val.workDoneProgress});
    if (val.showMessage)
        ret = json::addValue(ret, json::KV{"showMessage", val.showMessage});
    if (val.showDocument)
        ret = json::addValue(ret, json::KV{"showDocument", val.showDocument});
    return json::addScope(ret);
}

std::string toJsonValueStr(const StaleRequestSupport &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"cancel", val.cancel});
    ret = json::addValue(ret,json::KV{"retryOnContentModified", val.retryOnContentModified});
    return json::addScope(ret);
}

std::string toJsonValueStr(const RegularExpressionsClientCapabilities &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"engine", val.engine});
    if (val.version)
        ret = json::addValue(ret, json::KV{"version", val.version});
    return json::addScope(ret);
}

std::string toJsonValueStr(const MarkdownClientCapabilities &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"parser", val.parser});
    if (val.version)
        ret = json::addValue(ret, json::KV{"version", val.version});
    if (val.allowedTags)
        ret = json::addValue(ret, json::KV{"allowedTags", val.allowedTags});
    return json::addScope(ret);
}

std::string toJsonValueStr(const General &val)
{
    std::string ret;
    if (val.staleRequestSupport)
        ret = json::addValue(ret, json::KV{"staleRequestSupport", val.staleRequestSupport});
    if (val.regularExpressions)
        ret = json::addValue(ret, json::KV{"regularExpressions", val.regularExpressions});
    if (val.markdown)
        ret = json::addValue(ret, json::KV{"markdown", val.markdown});
    if (val.positionEncodings)
        ret = json::addValue(ret, json::KV{"positionEncodings", val.positionEncodings});
    if (val.experimental)
        ret = json::addValue(ret, json::KV{"experimental", std::string("{}")});
    return json::addScope(ret);
}

std::string toJsonValueStr(const ClientCapabilities &val)
{
    std::string ret;
    if (val.workspace)
        ret = json::addValue(ret, json::KV{"workspace", val.workspace});
    if (val.textDocument)
        ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    if (val.notebookDocument)
        ret = json::addValue(ret, json::KV{"notebookDocument", val.notebookDocument});
    if (val.window)
        ret = json::addValue(ret, json::KV{"window", val.window});
    if (val.general)
        ret = json::addValue(ret, json::KV{"general", val.general});
    return json::addScope(ret);
}

std::string toJsonValueStr(const WorkspaceFolder &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"uri", val.uri});
    ret = json::addValue(ret, json::KV{"name", val.name});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentSymbolClientCapabilities::TagSupport &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"valueSet", val.valueSet});
    return json::addScope(ret);
}

std::string toJsonValueStr(const PublishDiagnosticsClientCapabilities::TagSupport &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"valueSet", val.valueSet});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CompletionClientCapabilities::CompletionItem::InsertTextModeSupport &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"valueSet", val.valueSet});
    return json::addScope(ret);
}

std::string toJsonValueStr(const std::vector<WorkspaceFolder> &val)
{
    std::string ret;
    ret += "[";
    int valSize = val.size();
    for (int i = 0; i < valSize; i++) {
        ret += toJsonValueStr(val[i]);
        if (i < valSize -1) {
            ret += ",";
        }
    }
    ret += "]";
    return ret;
}

std::string toJsonValueStr(const InitializeParams &val)
{
    std::string ret;

    if (val.workDoneToken)
        ret += json::addValue(ret, json::KV{"workDoneToken", val.workDoneToken});
    if (val.language)
        ret = json::addValue(ret, json::KV{"language", val.language});
    if (val.processId)
        ret = json::addValue(ret, json::KV{"processId", val.processId});
    if (val.clientInfo)
        ret = json::addValue(ret, json::KV{"clientInfo", val.clientInfo});
    if (val.locale)
        ret = json::addValue(ret, json::KV{"locale", val.locale});
    if (val.rootPath)
        ret = json::addValue(ret, json::KV{"rootPath", val.rootPath});
    if (val.rootUri)
        ret = json::addValue(ret, json::KV{"rootUri", val.rootUri});
    if (val.initializationOptions)
        ret = json::addValue(ret, json::KV{"initializationOptions", std::string("{}")});

    ret = json::addValue(ret, json::KV{"capabilities", val.capabilities});

    if (val.trace)
        ret = json::addValue(ret, json::KV{"trace", val.trace});
    if (val.workspaceFolders)
        ret = json::addValue(ret, json::KV{"workspaceFolders", val.workspaceFolders});

    return json::addScope(ret);
}

std::string toJsonValueStr(const InitializedParams &val)
{
    std::string result;
    return json::addScope(result);
}

} // newlsp

