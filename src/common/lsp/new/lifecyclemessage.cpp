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
#include "lifecyclemessage.h"

namespace newlsp {

std::string ResolveSupport::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"properties", formatValue(properties)});
    return addScope(ret);
}

std::string CompletionClientCapabilities::CompletionItem::InsertTextModeSupport::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"valueSet", formatValue(valueSet)});
    return addScope(ret);
}

std::string CompletionClientCapabilities::CompletionList::toStdString() const
{
    std::string ret;
    if (itemDefaults)
        ret = addValue(ret, {"itemDefaults", formatValue(itemDefaults.value())});
    return addScope(ret);
}

std::string CompletionClientCapabilities::CompletionItemKind::toStdString() const
{
    std::string ret;
    if (valueSet)
        ret = addValue(ret, {"valueSet", formatValue(valueSet.value())});
    return addScope(ret);
}

std::string CompletionClientCapabilities::CompletionItem::TagSupport::toStdString() const
{
    return addScope(addValue({}, {"valueSet", formatValue(valueSet)}));
}

std::string CompletionClientCapabilities::CompletionItem::toStdString() const
{
    std::string ret;
    if (snippetSupport)
        ret = addValue(ret, {
                           "snippetSupport",
                           formatValue(snippetSupport.value())
                       });


    if (commitCharactersSupport)
        ret = addValue(ret, {
                           "commitCharactersSupport",
                           formatValue(commitCharactersSupport.value())
                       });

    if (documentationFormat)
        ret = addValue(ret, {
                           "documentationFormat",
                           formatValue(documentationFormat.value())
                       });

    if (deprecatedSupport)
        ret = addValue(ret, {
                           "deprecatedSupport",
                           formatValue(deprecatedSupport.value())
                       });

    if (preselectSupport)
        ret = addValue(ret, {
                           "preselectSupport",
                           formatValue(preselectSupport.value())
                       });

    if (tagSupport)
        ret = addValue(ret, {"tagSupport", tagSupport.value().toStdString()});

    if (insertReplaceSupport)
        ret = addValue(ret, {
                           "insertReplaceSupport",
                           formatValue(insertReplaceSupport.value())
                       });

    if (resolveSupport)
        ret = addValue(ret, {
                           "resolveSupport",
                           resolveSupport.value().toStdString()
                       });

    if (insertTextModeSupport)
        ret = addValue(ret, {
                           "insertTextModeSupport",
                           insertTextModeSupport.value().toStdString()
                       });

    if (labelDetailsSupport)
        ret = addValue(ret, {
                           "labelDetailsSupport",
                           formatValue(labelDetailsSupport.value())
                       });

    return addScope(ret);
}

std::string CodeActionLiteralSupport::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"codeActionKind", codeActionKind.toStdString()});
    return addScope(ret);
}

std::string ParameterInformation::toStdString() const
{
    std::string ret;
    if (labelOffsetSupport)
        ret = addValue(ret, {
                           "labelOffsetSupport",
                           formatValue(labelOffsetSupport.value())
                       });
    return addScope(ret);
}

std::string SignatureInformation::toStdString() const
{
    std::string ret;

    if (documentationFormat)
        ret = addValue(ret, {
                           "documentationFormat",
                           formatValue(documentationFormat.value())
                       });

    if (parameterInformation)
        ret = addValue(ret, {
                           "parameterInformation",
                           parameterInformation.value().toStdString()
                       });

    if (activeParameterSupport)
        ret = addValue(ret, {"activeParameterSupport",
                             formatValue(activeParameterSupport.value())});

    return addScope(ret);
}

std::string CodeActionKind::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"valueSet", formatValue(valueSet)});
    return addScope(ret);
}

std::string ChangeAnotationSupport::toStdString() const
{
    std::string ret;
    if (groupsOnLabel)
        ret = addValue(ret, {"groupsOnLabel", formatValue(groupsOnLabel.value())});
    return addScope(ret);
}

std::string SymbolKind::toStdString() const
{
    return addScope(addValue({}, {"valueSet", formatValue(valueSet)}));
}

std::string FoldingRangeKind::toStdString() const
{
    std::string ret;
    if (valueSet)
        ret = addValue(ret, {"valueSet", formatValue(valueSet.value())});
    return addScope(ret);
}

std::string FoldingRange::toStdString() const
{
    std::string ret;
    if (collapsedText)
        ret = addValue(ret, {"collapsedText", formatValue(collapsedText.value())});
    return addScope(ret);
}

std::string WorkspaceEditClientCapabilities::toStdString() const
{
    std::string ret;
    if (documentChanges)
        ret = addValue(ret, {"documentChanges", formatValue(documentChanges.value())});
    if (resourceOperations)
        ret = addValue(ret, {"resourceOperations", formatValue(resourceOperations.value())});
    if (failureHandling)
        ret = addValue(ret, {"failureHandling", formatValue(failureHandling.value())});
    if (normalizesLineEndings)
        ret = addValue(ret, {"normalizesLineEndings", formatValue(normalizesLineEndings.value())});
    if (changeAnnotationSupport)
        ret = addValue(ret, {"changeAnnotationSupport", changeAnnotationSupport.value().toStdString()});
    return addScope(ret);
}

std::string DidChangeConfigurationClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string DidChangeWatchedFilesClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (relativePatternSupport)
        ret = addValue(ret, {"relativePatternSupport", formatValue(relativePatternSupport.value())});
    return addScope(ret);
}

std::string ExecuteCommandClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string WorkspaceSymbolClientCapabilities::TagSupport::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"valueSet", formatValue(valueSet)});
    return addScope(ret);
}

std::string WorkspaceSymbolClientCapabilities::toStdString() const{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (symbolKind)
        ret = addValue(ret, {"symbolKind", symbolKind.value().toStdString()});
    if (tagSupport)
        ret = addValue(ret, {"tagSupport", tagSupport.value().toStdString()});
    if (resolveSupport)
        ret = addValue(ret, {"resolveSupport", resolveSupport.value()});
    return addScope(ret);
}

std::string SemanticTokensWorkspaceClientCapabilities::toStdString() const
{
    std::string ret;
    if (refreshSupport.value())
        ret = addValue(ret, {"refreshSupport", formatValue(refreshSupport.value())});
    return addScope(ret);
}

std::string CodeLensWorkspaceClientCapabilities::toStdString() const
{
    std::string ret;
    if (refreshSupport)
        ret = addValue(ret, {"refreshSupport", formatValue(refreshSupport.value())});
    return addScope(ret);
}

std::string InlineValueWorkspaceClientCapabilities::toStdString() const
{
    std::string ret;
    if (refreshSupport)
        ret = addValue(ret, {"refreshSupport", formatValue(refreshSupport.value())});
    return addScope(ret);
}

std::string InlayHintWorkspaceClientCapabilities::toStdString() const
{
    std::string ret;
    if (refreshSupport)
        ret = addValue(ret, {"refreshSupport", formatValue(refreshSupport.value())});
    return addScope(ret);
}

std::string DiagnosticWorkspaceClientCapabilities::toStdString() const
{
    std::string ret;
    if (refreshSupport)
        ret = addValue(ret, {"refreshSupport", formatValue(refreshSupport.value())});
    return addScope(ret);
}

std::string TextDocumentSyncClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (willSave)
        ret = addValue(ret, {"willSave", formatValue(willSave.value())});
    if (willSaveWaitUntil)
        ret = addValue(ret, {"willSaveWaitUntil", formatValue(willSaveWaitUntil.value())});
    if (didSave)
        ret = addValue(ret, {"didSave", formatValue(didSave.value())});
    return addScope(ret);
}

std::string CompletionClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (completionItem)
        ret = addValue(ret, {"completionItem", completionItem.value().toStdString()});
    if (completionItemKind)
        ret = addValue(ret, {"completionItemKind", completionItemKind.value().toStdString()});
    if (contextSupport)
        ret = addValue(ret, {"contextSupport", formatValue(contextSupport.value())});
    if (insertTextMode)
        ret = addValue(ret, {"insertTextMode", formatValue(insertTextMode.value())});
    if (itemDefaults)
        ret = addValue(ret, {"itemDefaults", itemDefaults.value().toStdString()});
    return addScope(ret);
}

std::string HoverClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (contentFormat)
        ret = addValue(ret, {"contentFormat", formatValue(contentFormat.value())});
    return addScope(ret);
}

std::string SignatureHelpClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (signatureInformation)
        ret = addValue(ret, {"signatureInformation", signatureInformation.value().toStdString()});
    if (contextSupport)
        ret = addValue(ret, {"contextSupport", formatValue(contextSupport.value())});
    return addScope(ret);
}

std::string DeclarationClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (linkSupport)
        ret = addValue(ret, {"linkSupport", formatValue(linkSupport.value())});
    return addScope(ret);
}

std::string DefinitionClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (linkSupport)
        ret = addValue(ret, {"linkSupport", formatValue(linkSupport.value())});
    return addScope(ret);
}

std::string TypeDefinitionClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (linkSupport)
        ret = addValue(ret, {"linkSupport", formatValue(linkSupport.value())});
    return addScope(ret);
}

std::string ImplementationClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (linkSupport)
        ret = addValue(ret, {"linkSupport", formatValue(linkSupport.value())});
    return addScope(ret);
}

std::string ReferenceClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string DocumentHighlightClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string DocumentSymbolClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (symbolKind)
        ret = addValue(ret, {"symbolKind", symbolKind.value().toStdString()});
    if (hierarchicalDocumentSymbolSupport)
        ret = addValue(ret, {"hierarchicalDocumentSymbolSupport",
                             formatValue(hierarchicalDocumentSymbolSupport.value())});
    if (tagSupport)
        ret = addValue(ret, {"tagSupport", tagSupport.value().toStdString()});
    if (labelSupport)
        ret = addValue(ret, {"labelSupport", formatValue(labelSupport.value())});
    return addScope(ret);
}

std::string CodeActionClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (codeActionLiteralSupport)
        ret = addValue(ret, {"codeActionLiteralSupport", codeActionLiteralSupport.value().toStdString()});
    if (isPreferredSupport)
        ret = addValue(ret, {"isPreferredSupport", formatValue(isPreferredSupport.value())});
    if (disabledSupport)
        ret = addValue(ret, {"disabledSupport", formatValue(disabledSupport.value())});
    if (dataSupport)
        ret = addValue(ret, {"dataSupport", formatValue(dataSupport.value())});
    if (resolveSupport)
        ret = addValue(ret, {"resolveSupport", resolveSupport.value().toStdString()});
    if (honorsChangeAnnotations)
        ret = addValue(ret, {"honorsChangeAnnotations", formatValue(honorsChangeAnnotations.value())});
    return addScope(ret);
}

std::string CodeLensClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string DocumentLinkClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (tooltipSupport)
        ret = addValue(ret, {"tooltipSupport", formatValue(tooltipSupport.value())});
    return addScope(ret);
}

std::string DocumentColorClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string DocumentFormattingClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string DocumentRangeFormattingClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string DocumentOnTypeFormattingClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string RenameClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (prepareSupport)
        ret = addValue(ret, {"prepareSupport", formatValue(prepareSupport.value())});
    if (prepareSupportDefaultBehavior)
        ret = addValue(ret, {"prepareSupportDefaultBehavior", formatValue(prepareSupportDefaultBehavior.value())});
    if (honorsChangeAnnotations)
        ret = addValue(ret, {"honorsChangeAnnotations", formatValue(honorsChangeAnnotations.value())});
    return addScope(ret);
}

std::string PublishDiagnosticsClientCapabilities::toStdString() const
{
    std::string ret;
    if (relatedInformation)
        ret = addValue(ret, {"relatedInformation", formatValue(relatedInformation.value())});
    if (tagSupport)
        ret = addValue(ret, {"tagSupport", tagSupport.value().toStdString()});
    if (versionSupport)
        ret = addValue(ret, {"versionSupport", formatValue(versionSupport.value())});
    if (codeDescriptionSupport)
        ret = addValue(ret, {"codeDescriptionSupport", formatValue(codeDescriptionSupport.value())});
    if (dataSupport)
        ret = addValue(ret, {"dataSupport", formatValue(dataSupport.value())});
    return addScope(ret);
}

std::string FoldingRangeClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (rangeLimit)
        ret = addValue(ret, {"rangeLimit", formatValue(rangeLimit.value())});
    if (lineFoldingOnly)
        ret = addValue(ret, {"lineFoldingOnly", formatValue(lineFoldingOnly.value())});
    if (foldingRangeKind)
        ret = addValue(ret, {"foldingRangeKind", foldingRangeKind.value().toStdString()});
    if (foldingRange)
        ret = addValue(ret, {"foldingRange", foldingRange.value().toStdString()});
    return addScope(ret);
}

std::string SelectionRangeClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string LinkedEditingRangeClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string CallHierarchyClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string SemanticTokensClientCapabilities::Requests::Full::toStdString() const
{
    std::string ret;
    if (delta) {
        ret = addValue(ret, {"delta", formatValue(delta.value())});
    }
    return addScope(ret);
}

std::string SemanticTokensClientCapabilities::Requests::toStdString() const
{
    std::string ret;
    if (range) {
        if (any_contrast<bool>(range.value())) {
            ret = addValue(ret, {"range", formatValue(std::any_cast<bool>(range.value()))});
        }
    }
    if (full) {
        ret = addValue(ret, {"full", full.value().toStdString()});
    }
    return addScope(ret);
}

std::string SemanticTokensClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    ret = addValue(ret, {"requests", requests.toStdString()});
    ret = addValue(ret, {"tokenTypes", formatValue(tokenTypes)});
    ret = addValue(ret, {"tokenModifiers", formatValue(tokenModifiers)});
    ret = addValue(ret, {"formats", formatValue(formats)});
    if (overlappingTokenSupport)
        ret = addValue(ret, {"overlappingTokenSupport", formatValue(overlappingTokenSupport.value())});
    if (multilineTokenSupport)
        ret = addValue(ret, {"multilineTokenSupport", formatValue(multilineTokenSupport.value())});
    if (serverCancelSupport)
        ret = addValue(ret, {"serverCancelSupport", formatValue(multilineTokenSupport.value())});
    if (augmentsSyntaxTokens)
        ret = addValue(ret, {"augmentsSyntaxTokens", formatValue(multilineTokenSupport.value())});
    return addScope(ret);
}



std::string MonikerClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string TypeHierarchyClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string InlineValueClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return addScope(ret);
}

std::string InlayHintClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (resolveSupport)
        ret = addValue(ret, {"resolveSupport", resolveSupport.value().toStdString()});
    return addScope(ret);
}

std::string DiagnosticClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (relatedDocumentSupport)
        ret = addValue(ret, {"relatedDocumentSupport", formatValue(relatedDocumentSupport.value())});
    return addScope(ret);
}

std::string TextDocumentClientCapabilities::toStdString() const {
    std::string ret;
    if (synchronization)
        ret = addValue(ret, {"synchronization", synchronization.value().toStdString()});
    if (completion)
        ret = addValue(ret, {"completion", completion.value().toStdString()});
    if (hover)
        ret = addValue(ret, {"hover", hover.value().toStdString()});
    if (signatureHelp)
        ret = addValue(ret, {"signatureHelp", signatureHelp.value().toStdString()});
    if (declaration)
        ret = addValue(ret, {"declaration", declaration.value().toStdString()});
    if (definition)
        ret = addValue(ret, {"definition", definition.value().toStdString()});
    if (typeDefinition)
        ret = addValue(ret, {"typeDefinition", typeDefinition.value().toStdString()});
    if (implementation)
        ret = addValue(ret, {"implementation", implementation.value().toStdString()});
    if (references)
        ret = addValue(ret, {"references", references.value().toStdString()});
    if (documentHighlight)
        ret = addValue(ret, {"documentHighlight", documentHighlight.value().toStdString()});
    if (documentSymbol)
        ret = addValue(ret, {"documentSymbol", documentSymbol.value().toStdString()});
    if (codeAction)
        ret = addValue(ret, {"codeAction", codeAction.value().toStdString()});
    if (codeLens)
        ret = addValue(ret, {"codeLens", codeLens.value().toStdString()});
    if (documentLink)
        ret = addValue(ret, {"documentLink", documentLink.value().toStdString()});
    if (colorProvider)
        ret = addValue(ret, {"colorProvider", colorProvider.value().toStdString()});
    if (formatting)
        ret = addValue(ret, {"formatting", formatting.value().toStdString()});
    if (rangeFormatting)
        ret = addValue(ret, {"rangeFormatting", formatting.value().toStdString()});
    if (onTypeFormatting)
        ret = addValue(ret, {"onTypeFormatting", onTypeFormatting.value().toStdString()});
    if (rename)
        ret = addValue(ret, {"rename", rename.value().toStdString()});
    if (publishDiagnostics)
        ret = addValue(ret, {"publishDiagnostics", publishDiagnostics.value().toStdString()});
    if (foldingRange)
        ret = addValue(ret, {"foldingRange", foldingRange.value().toStdString()});
    if (selectionRange)
        ret = addValue(ret, {"selectionRange", selectionRange.value().toStdString()});
    if (linkedEditingRange)
        ret = addValue(ret, {"linkedEditingRange", linkedEditingRange.value().toStdString()});
    if (callHierarchy)
        ret = addValue(ret, {"callHierarchy", callHierarchy.value().toStdString()});
    if (semanticTokens)
        ret = addValue(ret, {"semanticTokens", semanticTokens.value().toStdString()});
    if (moniker)
        ret = addValue(ret, {"moniker", moniker.value().toStdString()});
    if (typeHierarchy)
        ret = addValue(ret, {"typeHierarchy", typeHierarchy.value().toStdString()});
    if (inlineValue)
        ret = addValue(ret, {"inlineValue", inlineValue.value().toStdString()});
    if (inlayHint)
        ret = addValue(ret, {"inlayHint", inlayHint.value().toStdString()});
    if (diagnostic)
        ret = addValue(ret, {"diagnostic", diagnostic.value().toStdString()});
    return addScope(ret);
}

std::string FileOperations::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (didCreate)
        ret = addValue(ret, {"didCreate", formatValue(didCreate.value())});
    if (willCreate)
        ret = addValue(ret, {"willCreate", formatValue(willCreate.value())});
    if (didRename)
        ret = addValue(ret, {"didRename", formatValue(didRename.value())});
    if (willRename)
        ret = addValue(ret, {"willRename", formatValue(willRename.value())});
    if (didDelete)
        ret = addValue(ret, {"didDelete", formatValue(didDelete.value())});
    if (willDelete)
        ret = addValue(ret, {"willDelete", formatValue(willDelete.value())});
    return addScope(ret);
}

std::string Workspace::toStdString() const
{
    std::string ret;
    if (applyEdit)
        ret = addValue(ret, {"applyEdit", formatValue(applyEdit.value())});
    if (workspaceEdit)
        ret = addValue(ret, {"workspaceEdit", workspaceEdit.value().toStdString()});
    if (didChangeConfiguration)
        ret = addValue(ret, {"didChangeConfiguration", didChangeConfiguration.value().toStdString()});
    if (didChangeWatchedFiles)
        ret = addValue(ret, {"didChangeWatchedFiles", didChangeWatchedFiles->toStdString()});
    if (symbol)
        ret = addValue(ret, {"symbol", symbol.value().toStdString()});
    if (executeCommand)
        ret = addValue(ret, {"executeCommand", executeCommand.value().toStdString()});
    if (workspaceFolders)
        ret = addValue(ret, {"workspaceFolders", formatValue(workspaceFolders.value())});
    if (configuration)
        ret = addValue(ret, {"configuration", formatValue(configuration.value())});
    if (semanticTokens)
        ret = addValue(ret, {"semanticTokens", semanticTokens.value().toStdString()});
    if (codeLens)
        ret = addValue(ret, {"codeLens", codeLens.value().toStdString()});
    if (fileOperations)
        ret = addValue(ret, {"fileOperations", fileOperations.value().toStdString()});
    if (inlineValue)
        ret = addValue(ret, {"inlineValue", inlineValue.value().toStdString()});
    if (inlayHint)
        ret = addValue(ret, {"inlayHint", inlayHint.value().toStdString()});
    if (diagnostics)
        ret = addValue(ret, {"diagnostics", diagnostics.value().toStdString()});
    return addScope(ret);
}

std::string ClientInfo::toStdString() const
{
    std::string result{};
    result = addValue(result, {"name", formatValue(name)});
    if (version)
        result = addValue(result, {"version", formatValue(version.value())});
    return addScope(result);
}

std::string NotebookDocumentSyncClientCapabilities::toStdString() const
{
    std::string ret = "";
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (executionSummarySupport)
        ret = addValue(ret, {"executionSummarySupport", formatValue(executionSummarySupport.value())});
    return addScope(ret);
}

std::string NotebookDocumentClientCapabilities::toStdString() const
{
    std::string ret;
    ret = addValue({}, {"synchronization", synchronization.toStdString()});
    return addScope(ret);
}

std::string MessageActionItem::toStdString() const
{
    std::string ret;
    if (additionalPropertiesSupport)
        ret = addValue(ret, {"additionalPropertiesSupport", formatValue(additionalPropertiesSupport.value())});
    return addScope(ret);
}

std::string ShowMessageRequestClientCapabilities::toStdString() const
{
    std::string ret;
    if (messageActionItem)
        ret = addValue(ret, {"messageActionItem", messageActionItem.value().toStdString()});
    return addScope(ret);
}

std::string ShowDocumentClientCapabilities::toStdString() const {
    std::string ret;
    ret = addValue(ret, {"support", formatValue(support)});
    return addScope(ret);
}

std::string Window::toStdString() const
{
    std::string ret;
    if (workDoneProgress)
        ret = addValue(ret, {"workDoneProgress",formatValue(workDoneProgress.value())});
    if (showMessage)
        ret = addValue(ret, {"showMessage", showMessage.value().toStdString()});
    if (showDocument)
        ret = addValue(ret, {"showDocument", showDocument.value().toStdString()});
    return addScope(ret);
}

std::string StaleRequestSupport::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"cancel", formatValue(cancel)});
    ret = addValue(ret,{"retryOnContentModified", formatValue(retryOnContentModified)});
    return addScope(ret);
}

std::string RegularExpressionsClientCapabilities::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"engine", formatValue(engine)});
    if (version)
        ret = addValue(ret, {"version", formatValue(version.value())});
    return addScope(ret);
}

std::string MarkdownClientCapabilities::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"parser", formatValue(parser)});
    if (version)
        ret = addValue(ret, {"version", formatValue(version.value())});
    if (allowedTags)
        ret = addValue(ret, {"allowedTags", formatValue(allowedTags.value())});
    return addScope(ret);
}

std::string General::toStdString() const
{
    std::string ret;
    if (staleRequestSupport)
        ret = addValue(ret, {"staleRequestSupport", staleRequestSupport.value().toStdString()});
    if (regularExpressions)
        ret = addValue(ret, {"regularExpressions", regularExpressions.value().toStdString()});
    if (markdown)
        ret = addValue(ret, {"markdown", markdown.value().toStdString()});
    if (positionEncodings)
        ret = addValue(ret, {"positionEncodings", formatValue(positionEncodings.value())});
    if (experimental)
        ret = addValue(ret, {"experimental", formatValue(std::string{"{}"})});
    return addScope(ret);
}

std::string ClientCapabilities::toStdString() const
{
    std::string ret;
    if (workspace)
        ret = addValue(ret, {"workspace", workspace.value().toStdString()});
    if (textDocument)
        ret = addValue(ret, {"textDocument", textDocument.value().toStdString()});
    if (notebookDocument)
        ret = addValue(ret, {"notebookDocument", notebookDocument.value().toStdString()});
    if (window)
        ret = addValue(ret, {"window", window.value().toStdString()});
    if (general)
        ret = addValue(ret, {"general", general.value().toStdString()});
    return addScope(ret);
}

std::string WorkspaceFolder::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"uri", formatValue(uri)});
    ret = addValue(ret, {"name", formatValue(name)});
    return addScope(ret);
}

std::string DocumentSymbolClientCapabilities::TagSupport::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"valueSet", formatValue(valueSet)});
    return addScope(ret);
}

std::string PublishDiagnosticsClientCapabilities::TagSupport::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"valueSet", formatValue(valueSet)});
    return addScope(ret);
}

std::string InitializeParams::formatValue(const std::vector<WorkspaceFolder> &workspaceFolders) const
{
    std::string ret;
    ret += "[";
    for (int i = 0; i < workspaceFolders.size(); i++) {
        ret += workspaceFolders[i].toStdString();
        if (i < workspaceFolders.size() -1) {
            ret += ",";
        }
    }
    ret += "]";
    return ret;
}

std::string InitializeParams::toStdString() const
{
    std::string ret;

    if (WorkDoneProgressParams::workDoneToken)
        ret += WorkDoneProgressParams::toStdString();

    if (language)
        ret = addValue(ret, {"language", JsonConvert::formatValue(language.value())});
    if (processId)
        ret = addValue(ret, {"processId", processId.value()});
    if (clientInfo)
        ret = addValue(ret, {"clientInfo", clientInfo.value().toStdString()});
    if (locale)
        ret = addValue(ret, {"locale", JsonConvert::formatValue(locale.value())});
    if (rootPath)
        ret = addValue(ret, {"rootPath", JsonConvert::formatValue(rootPath.value())});
    if (rootUri)
        ret = addValue(ret, {"rootUri", JsonConvert::formatValue(rootUri.value())});
    if (initializationOptions)
        ret = addValue(ret, {"initializationOptions", "{}"});

    ret = addValue(ret, {"capabilities", capabilities.toStdString()});

    if (trace)
        ret = addValue(ret, {"trace", JsonConvert::formatValue(trace.value())});
    if (workspaceFolders)
        ret = addValue(ret, {"workspaceFolders", formatValue(workspaceFolders.value())});

    return addScope(ret);
}

std::string InitializedParams::toStdString() const
{
    std::string result;
    return addScope(result);
}

} // newlsp

