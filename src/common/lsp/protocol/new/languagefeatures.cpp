// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "languagefeatures.h"

namespace newlsp{

std::string toJsonValueStr(const newlsp::DeclarationParams &val)
{
    std::string ret = json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val)) });
    return ret;
}

std::string toJsonValueStr(const DefinitionParams &val)
{
    std::string ret = json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))});
    return ret;
}

std::string toJsonValueStr(const TypeDefinitionParams &val)
{
    std::string ret = json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))});
    return ret;
}

std::string toJsonValueStr(const ImplementationParams &val)
{
    std::string ret = json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))});
    return ret;
}

std::string toJsonValueStr(const ReferenceParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"context", val.context});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CallHierarchyPrepareParams &val)
{
    std::string ret = json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val))});
    return ret;
}

std::string toJsonValueStr(const CallHierarchyIncomingCallsParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"item", val.item});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CallHierarchyOutgoingCallsParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"item", val.item});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CallHierarchyItem &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"name", val.name});
    ret = json::addValue(ret, json::KV{"kind", val.kind});
    ret = json::addValue(ret, json::KV{"tags", val.tags});
    ret = json::addValue(ret, json::KV{"detail", val.detail});
    ret = json::addValue(ret, json::KV{"uri", val.uri});
    ret = json::addValue(ret, json::KV{"range", val.range});
    ret = json::addValue(ret, json::KV{"selectionRange", val.selectionRange});
    ret = json::addValue(ret, json::KV{"data", val.data});
    return json::addScope(ret);
}

std::string toJsonValueStr(const ReferenceParams::ReferenceContext &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"includeDeclaration", val.includeDeclaration});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TypeHierarchyPrepareParams &val)
{
    std::string ret = json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val))});
    return ret;
}

std::string toJsonValueStr(const TypeHierarchySupertypesParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"item", val.item});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TypeHierarchyItem &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"name", val.name});
    ret = json::addValue(ret, json::KV{"kind", val.kind});
    ret = json::addValue(ret, json::KV{"tags", val.tags});
    ret = json::addValue(ret, json::KV{"detail", val.detail});
    ret = json::addValue(ret, json::KV{"uri", val.uri});
    ret = json::addValue(ret, json::KV{"range", val.range});
    ret = json::addValue(ret, json::KV{"selectionRange", val.selectionRange});
    ret = json::addValue(ret, json::KV{"data", val.data});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TypeHierarchySubtypesParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"item", val.item});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentHighlightParams &val)
{
    std::string ret = json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))});
    return ret;
}

std::string toJsonValueStr(const DocumentLinkParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentLink &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"range", val.range});
    ret = json::addValue(ret, json::KV{"target", val.target});
    ret = json::addValue(ret, json::KV{"tooltip", val.tooltip});
    ret = json::addValue(ret, json::KV{"data", val.data});
    return json::addScope(ret);
}

std::string toJsonValueStr(const HoverParams &val)
{
    std::string ret = json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val))});
    return ret;
}

std::string toJsonValueStr(const CodeLensParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CodeLens &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"range", val.range});
    ret = json::addValue(ret, json::KV{"command", val.command});
    ret = json::addValue(ret, json::KV{"data", val.data});
    return json::addScope(ret);
}

std::string toJsonValueStr(const FoldingRangeParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SelectionRangeParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"positions", val.positions});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentSymbolParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SemanticTokensParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SemanticTokensDeltaParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"previousResultId", val.previousResultId});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SemanticTokensRangeParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"range", val.range});
    return json::addScope(ret);
}

std::string toJsonValueStr(const InlayHintParams &val)
{
    std::string ret = json::delScope(toJsonValueStr(WorkDoneProgressParams(val)));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"range", val.range});
    return json::addScope(ret);
}

std::string toJsonValueStr(const InlineValueParams &val)
{
    std::string ret = json::delScope(toJsonValueStr(WorkDoneProgressParams(val)));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"range", val.range});
    ret = json::addValue(ret, json::KV{"context", val.context});
    return json::addScope(ret);
}

std::string toJsonValueStr(const InlineValueContext &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"frameId", val.frameId});
    ret = json::addValue(ret, json::KV{"stoppedLocation", val.stoppedLocation});
    return  json::addScope(ret);
}

std::string toJsonValueStr(const MonikerParams &val)
{
    std::string ret = json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val)) });
    return ret;
}

std::string toJsonValueStr(const CompletionContext &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"triggerKind", val.triggerKind});
    ret = json::addValue(ret, json::KV{"triggerCharacter", val.triggerCharacter});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CompletionParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"context", val.context});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentDiagnosticParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"identifier", val.identifier});
    ret = json::addValue(ret, json::KV{"previousResultId", val.previousResultId});
    return json::addScope(ret);
}

std::string toJsonValueStr(const PreviousResultId &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"uri", val.uri});
    ret = json::addValue(ret, json::KV{"value", val.value});
    return json::addScope(ret);
}

std::string toJsonValueStr(const WorkspaceDiagnosticParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"identifier", val.identifier});
    ret = json::addValue(ret, json::KV{"previousResultIds", val.previousResultIds});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SignatureHelpParams::SignatureHelpContext::SignatureHelp::SignatureInformation::ParameterInformation &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"label", val.label});
    ret = json::addValue(ret, json::KV{"documentation", val.documentation});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SignatureHelpParams::SignatureHelpContext::SignatureHelp::SignatureInformation &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"label", val.label});
    ret = json::addValue(ret, json::KV{"parameters", val.parameters});
    ret = json::addValue(ret, json::KV{"documentation", val.documentation});
    ret = json::addValue(ret, json::KV{"activeParameter", val.activeParameter});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SignatureHelpParams::SignatureHelpContext::SignatureHelp &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"signatures", val.signatures});
    ret = json::addValue(ret, json::KV{"activeParameter", val.activeParameter});
    ret = json::addValue(ret, json::KV{"activeSignature", val.activeSignature});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SignatureHelpParams::SignatureHelpContext &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"isRetrigger", val.isRetrigger});
    ret = json::addValue(ret, json::KV{"triggerKind", val.triggerKind});
    ret = json::addValue(ret, json::KV{"triggerCharacter", val.triggerCharacter});
    ret = json::addValue(ret, json::KV{"activeSignatureHelp", val.activeSignatureHelp});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SignatureHelpParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val))}));
    ret = json::addValue(ret, json::KV{"isRetrigger", val.context});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CodeActionParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"range", val.range});
    ret = json::addValue(ret, json::KV{"context", val.context});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CodeAction::Disabled &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"reason", val.reason});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CodeAction &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"title", val.title});
    ret = json::addValue(ret, json::KV{"kind", val.kind});
    ret = json::addValue(ret, json::KV{"diagnostics", val.diagnostics});
    ret = json::addValue(ret, json::KV{"isPreferred", val.isPreferred});
    ret = json::addValue(ret, json::KV{"disabled", val.disabled});
    ret = json::addValue(ret, json::KV{"edit", val.edit});
    ret = json::addValue(ret, json::KV{"command", val.command});
    ret = json::addValue(ret, json::KV{"data", val.data});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CodeActionContext &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"only", val.only});
    ret = json::addValue(ret, json::KV{"diagnostics", val.diagnostics});
    ret = json::addValue(ret, json::KV{"triggerKind", val.triggerKind});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentColorParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    return json::addScope(ret);
}

std::string toJsonValueStr(const Color &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"red", val.red});
    ret = json::addValue(ret, json::KV{"blue", val.blue});
    ret = json::addValue(ret, json::KV{"green", val.green});
    ret = json::addValue(ret, json::KV{"alpha", val.alpha});
    return json::addScope(ret);
}

std::string toJsonValueStr(const ColorPresentationParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(WorkDoneProgressParams(val)),
     toJsonValueStr(PartialResultParams(val))}));
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"color", val.color});
    ret = json::addValue(ret, json::KV{"range", val.range});
    return json::addScope(ret);
}

std::string toJsonValueStr(const FormattingOptions &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"tabSize", val.tabSize});
    ret = json::addValue(ret, json::KV{"insertSpace", val.insertSpaces});
    ret = json::addValue(ret, json::KV{"trimTrailingWhitespace", val.trimTrailingWhitespace});
    ret = json::addValue(ret, json::KV{"insertFinalNewline", val.insertFinalNewline});
    ret = json::addValue(ret, json::KV{"trimFinalNewlines", val.trimFinalNewlines});
    for (auto &&pairVal : val) {
        if (any_contrast<bool>(pairVal.second)) {
            ret = json::addValue(ret, json::KV{pairVal.first, std::any_cast<bool>(pairVal.second)});
        } else if(any_contrast<int>(pairVal.second)) {
            ret = json::addValue(ret, json::KV{pairVal.first, std::any_cast<int>(pairVal.second)});
        } else if(any_contrast<std::string>(pairVal.second)) {
            ret = json::addValue(ret, json::KV{pairVal.first, std::any_cast<std::string>(pairVal.second)});
        }
    }
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentFormattingParams &val)
{
    std::string ret = json::delScope(toJsonValueStr(WorkDoneProgressParams(val)));
    ret = json::addValue(ret, json::KV{"options", val.options});
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentRangeFormattingParams &val)
{
    std::string ret = json::delScope(toJsonValueStr(WorkDoneProgressParams(val)));
    ret = json::addValue(ret, json::KV{"options", val.options});
    ret = json::addValue(ret, json::KV{"range", val.range});
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentOnTypeFormattingParams &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"options", val.options});
    ret = json::addValue(ret, json::KV{"ch", val.ch});
    ret = json::addValue(ret, json::KV{"position", val.position});
    return json::addScope(ret);
}

std::string toJsonValueStr(const RenameParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val))}));
    ret = json::addValue(ret, json::KV{"newName", val.newName});
    return json::addScope(ret);
}

std::string toJsonValueStr(const PrepareRenameParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val))}));
    return json::addScope(ret);
}

std::string toJsonValueStr(const LinkedEditingRangeParams &val)
{
    std::string ret = json::delScope(json::mergeObjs(
    {toJsonValueStr(TextDocumentPositionParams(val)),
     toJsonValueStr(WorkDoneProgressParams(val))}));
    return json::addScope(ret);
}

} // namespace newlsp
