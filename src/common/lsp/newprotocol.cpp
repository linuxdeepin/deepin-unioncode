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
#include "newprotocol.h"

std::string newlsp::Lifecycle::Initialize::JsonConvert::formatScope(const std::string &src)
{
    return "{" + src + "}";
}

std::string newlsp::Lifecycle::Initialize::JsonConvert::formatKey(const std::string &key)
{
    return "\"" + key + "\"";
}

std::string newlsp::Lifecycle::Initialize::JsonConvert::formatValue(unsigned int value)
{
    return std::to_string(value);
}

std::string newlsp::Lifecycle::Initialize::JsonConvert::formatValue(int value)
{
    return std::to_string(value);
}

std::string newlsp::Lifecycle::Initialize::JsonConvert::formatValue(bool value)
{
    if (true == value)
        return "true";
    if (false == value)
        return "false";
    return "false";
}

std::string newlsp::Lifecycle::Initialize::JsonConvert::formatValue(const std::string &value)
{
    return "\"" + value + "\"";
}

std::string newlsp::Lifecycle::Initialize::JsonConvert::formatValue(const std::vector<int> &vecInt)
{
    std::string ret;
    if (vecInt.size() < 0)
        return ret;

    ret += "[";
    for (int i = 0; i < vecInt.size(); i++) {
        ret += formatValue(vecInt[i]);
        if (i < vecInt.size() - 1)
            ret += ",";
    }
    ret += "]";

    return ret;
}

std::string newlsp::Lifecycle::Initialize::JsonConvert::formatValue(const std::vector<std::string> &vecString)
{
    std::string ret;
    if (vecString.size() < 0)
        return ret;

    ret += "[";
    for (int i = 0; i < vecString.size(); i++) {
        ret += formatValue(vecString[i]);
        if (i < vecString.size() - 1)
            ret += ",";
    }
    ret += "]";

    return ret;
}

std::string newlsp::Lifecycle::Initialize::JsonConvert::addValue(const std::string &src,
                                                              const std::pair<std::string,
                                                              std::any> &elem)
{
    std::string temp;
    if (elem.first.empty() || !elem.second.has_value())
        return temp;

    temp = formatKey(elem.first) + ":" ;
    if (any_contrast<std::string>(elem.second)) {
        temp += std::any_cast<std::string>(elem.second);
    } else if (any_contrast<int>(elem.second)) {
        temp += std::to_string(std::any_cast<int>(elem.second));
    } else if (any_contrast<unsigned int>(elem.second)) {
        temp += std::to_string(std::any_cast<unsigned int>(elem.second));
    } else if (any_contrast<bool>(elem.second)) {
        temp += std::to_string(std::any_cast<bool>(elem.second));
    }

    if (!src.empty())
        return src + "," + temp;
    else
        return temp;
}

std::string newlsp::Lifecycle::Initialize::JsonConvert::addValue(const std::string &src,
                                                              std::initializer_list<std::pair<std::string,
                                                              std::any>> &elems)
{
    auto ret = src;
    for (auto elem : elems) {
        ret = addValue(ret, elem);
    }
    return ret;
}

std::string newlsp::Lifecycle::Initialize::WorkDoneProgressParams::toStdString() const
{
    std::string ret;
    if (workDoneToken) {
        if (any_contrast<bool>(workDoneToken.value()))
            ret = addValue(ret, {"workDoneToken", formatValue(std::any_cast<bool>(workDoneToken.value()))});
        else if (any_contrast<std::string>(workDoneToken.value()))
            ret = addValue(ret, {"workDoneToken", formatValue(std::any_cast<std::string>(workDoneToken.value()))});
    }
    return ret;
}

std::string newlsp::Lifecycle::Initialize::ResolveSupport::toStdString() const
{
    return formatScope(
                addValue({}, {
                             "properties",
                             formatValue(properties)
                         })
                );
}

std::string newlsp::Lifecycle::Initialize::CompletionClientCapabilities::CompletionItem::InsertTextModeSupport::toStdString() const
{
    return formatScope(addValue({}, {"valueSet", formatValue(valueSet)}));
}

std::string newlsp::Lifecycle::Initialize::CompletionClientCapabilities::CompletionList::toStdString() const
{
    std::string ret;
    if (itemDefaults)
        ret = addValue(ret, {"itemDefaults", formatValue(itemDefaults.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::CompletionClientCapabilities::CompletionItemKind::toStdString() const
{
    std::string ret;
    if (valueSet)
        ret = addValue(ret, {"valueSet", formatValue(valueSet.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::CompletionClientCapabilities::CompletionItem::TagSupport::toStdString() const
{
    return formatScope(addValue({}, {"valueSet", formatValue(valueSet)}));
}

std::string newlsp::Lifecycle::Initialize::CompletionClientCapabilities::CompletionItem::toStdString() const
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
        ret = addValue(ret, {
                           "tagSupport",
                           tagSupport.value().toStdString()
                       });

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

    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::CodeActionLiteralSupport::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"codeActionKind", codeActionKind.toStdString()});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::ParameterInformation::toStdString() const
{
    std::string ret;
    if (labelOffsetSupport)
        ret = addValue(ret, {
                           "labelOffsetSupport",
                           formatValue(labelOffsetSupport.value())
                       });
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::SignatureInformation::toStdString() const
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

    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::CodeActionKind::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"valueSet", formatValue(valueSet)});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::ChangeAnotationSupport::toStdString() const
{
    std::string ret;
    if (groupsOnLabel)
        ret = addValue(ret, {"groupsOnLabel", formatValue(groupsOnLabel.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::SymbolKind::toStdString() const
{
    return formatScope(addValue({}, {"valueSet", formatValue(valueSet)}));
}

std::string newlsp::Lifecycle::Initialize::FoldingRangeKind::toStdString() const
{
    std::string ret;
    if (valueSet)
        ret = addValue(ret, {"valueSet", formatValue(valueSet.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::FoldingRange::toStdString() const
{
    std::string ret;
    if (collapsedText)
        ret = addValue(ret, {"collapsedText", formatValue(collapsedText.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::WorkspaceEditClientCapabilities::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DidChangeConfigurationClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DidChangeWatchedFilesClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (relativePatternSupport)
        ret = addValue(ret, {"relativePatternSupport", formatValue(relativePatternSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::ExecuteCommandClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::WorkspaceSymbolClientCapabilities::TagSupport::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"valueSet", formatValue(valueSet)});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::WorkspaceSymbolClientCapabilities::toStdString() const{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (symbolKind)
        ret = addValue(ret, {"symbolKind", symbolKind.value().toStdString()});
    if (tagSupport)
        ret = addValue(ret, {"tagSupport", tagSupport.value().toStdString()});
    if (resolveSupport)
        ret = addValue(ret, {"resolveSupport", resolveSupport.value()});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::SemanticTokensWorkspaceClientCapabilities::toStdString() const
{
    std::string ret;
    if (refreshSupport.value())
        ret = addValue(ret, {"refreshSupport", formatValue(refreshSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::CodeLensWorkspaceClientCapabilities::toStdString() const
{
    std::string ret;
    if (refreshSupport)
        ret = addValue(ret, {"refreshSupport", formatValue(refreshSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::InlineValueWorkspaceClientCapabilities::toStdString() const
{
    std::string ret;
    if (refreshSupport)
        ret = addValue(ret, {"refreshSupport", formatValue(refreshSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::InlayHintWorkspaceClientCapabilities::toStdString() const
{
    std::string ret;
    if (refreshSupport)
        ret = addValue(ret, {"refreshSupport", formatValue(refreshSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DiagnosticWorkspaceClientCapabilities::toStdString() const
{
    std::string ret;
    if (refreshSupport)
        ret = addValue(ret, {"refreshSupport", formatValue(refreshSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::TextDocumentSyncClientCapabilities::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::CompletionClientCapabilities::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::HoverClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (contentFormat)
        ret = addValue(ret, {"contentFormat", formatValue(contentFormat.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::SignatureHelpClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (signatureInformation)
        ret = addValue(ret, {"signatureInformation", signatureInformation.value().toStdString()});
    if (contextSupport)
        ret = addValue(ret, {"contextSupport", formatValue(contextSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DeclarationClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (linkSupport)
        ret = addValue(ret, {"linkSupport", formatValue(linkSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DefinitionClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (linkSupport)
        ret = addValue(ret, {"linkSupport", formatValue(linkSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::TypeDefinitionClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (linkSupport)
        ret = addValue(ret, {"linkSupport", formatValue(linkSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::ImplementationClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (linkSupport)
        ret = addValue(ret, {"linkSupport", formatValue(linkSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::ReferenceClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DocumentHighlightClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DocumentSymbolClientCapabilities::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::CodeActionClientCapabilities::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::CodeLensClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DocumentLinkClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (tooltipSupport)
        ret = addValue(ret, {"tooltipSupport", formatValue(tooltipSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DocumentColorClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DocumentFormattingClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DocumentRangeFormattingClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DocumentOnTypeFormattingClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::RenameClientCapabilities::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::PublishDiagnosticsClientCapabilities::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::FoldingRangeClientCapabilities::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::SelectionRangeClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::LinkedEditingRangeClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::CallHierarchyClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::SemanticTokensClientCapabilities::Requests::Full::toStdString() const
{
    std::string ret;
    if (delta) {
        ret = addValue(ret, {"delta", formatValue(delta.value())});
    }
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::SemanticTokensClientCapabilities::Requests::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::SemanticTokensClientCapabilities::toStdString() const
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
    return formatScope(ret);
}



std::string newlsp::Lifecycle::Initialize::MonikerClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::TypeHierarchyClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::InlineValueClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::InlayHintClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (resolveSupport)
        ret = addValue(ret, {"resolveSupport", resolveSupport.value().toStdString()});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DiagnosticClientCapabilities::toStdString() const
{
    std::string ret;
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (relatedDocumentSupport)
        ret = addValue(ret, {"relatedDocumentSupport", formatValue(relatedDocumentSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::TextDocumentClientCapabilities::toStdString() const {
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::FileOperations::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::Workspace::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::ClientInfo::toStdString() const
{
    std::string result{};
    result = addValue(result, {"name", formatValue(name)});
    if (version)
        result = addValue(result, {"version", formatValue(version.value())});
    return formatScope(result);
}

std::string newlsp::Lifecycle::Initialize::NotebookDocumentSyncClientCapabilities::toStdString() const
{
    std::string ret = "";
    if (dynamicRegistration)
        ret = addValue(ret, {"dynamicRegistration", formatValue(dynamicRegistration.value())});
    if (executionSummarySupport)
        ret = addValue(ret, {"executionSummarySupport", formatValue(executionSummarySupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::NotebookDocumentClientCapabilities::toStdString() const
{
    std::string ret;
    ret = addValue({}, {"synchronization", synchronization.toStdString()});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::MessageActionItem::toStdString() const
{
    std::string ret;
    if (additionalPropertiesSupport)
        ret = addValue(ret, {"additionalPropertiesSupport", formatValue(additionalPropertiesSupport.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::ShowMessageRequestClientCapabilities::toStdString() const
{
    std::string ret;
    if (messageActionItem)
        ret = addValue(ret, {"messageActionItem", messageActionItem.value().toStdString()});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::ShowDocumentClientCapabilities::toStdString() const {
    std::string ret;
    ret = addValue(ret, {"support", formatValue(support)});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::Window::toStdString() const
{
    std::string ret;
    if (workDoneProgress)
        ret = addValue(ret, {"workDoneProgress",formatValue(workDoneProgress.value())});
    if (showMessage)
        ret = addValue(ret, {"showMessage", showMessage.value().toStdString()});
    if (showDocument)
        ret = addValue(ret, {"showDocument", showDocument.value().toStdString()});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::StaleRequestSupport::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"cancel", formatValue(cancel)});
    ret = addValue(ret,{"retryOnContentModified", formatValue(retryOnContentModified)});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::RegularExpressionsClientCapabilities::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"engine", formatValue(engine)});
    if (version)
        ret = addValue(ret, {"version", formatValue(version.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::MarkdownClientCapabilities::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"parser", formatValue(parser)});
    if (version)
        ret = addValue(ret, {"version", formatValue(version.value())});
    if (allowedTags)
        ret = addValue(ret, {"allowedTags", formatValue(allowedTags.value())});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::General::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::ClientCapabilities::toStdString() const
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
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::WorkspaceFolder::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"uri", formatValue(uri)});
    ret = addValue(ret, {"name", formatValue(name)});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::InitializeParams::formatValue(
        const std::vector<newlsp::Lifecycle::Initialize::WorkspaceFolder> &workspaceFolders) const
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

std::string newlsp::Lifecycle::Initialize::InitializeParams::toStdString() const
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

    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::DocumentSymbolClientCapabilities::TagSupport::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"valueSet", formatValue(valueSet)});
    return formatScope(ret);
}

std::string newlsp::Lifecycle::Initialize::PublishDiagnosticsClientCapabilities::TagSupport::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"valueSet", formatValue(valueSet)});
    return formatScope(ret);
}

