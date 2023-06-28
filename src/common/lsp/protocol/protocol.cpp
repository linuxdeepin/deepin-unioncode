// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "protocol.h"
#include "newprotocol.h"
#include "common/util/processutil.h"

#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QQueue>
#include <QUrl>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

namespace lsp {

const QString C_CPP{"C/C++"};
const QString JAVA{"Java"};
const QString PYTHON{"Python"};
const QString JS{"JS"};
const QString K_ID {"id"};
const QString K_JSON_RPC {"jsonrpc"};
const QString K_METHOD {"method"};
const QString K_RESULT {"result"};
const QString K_PARAMS {"params"};
const QString K_CAPABILITIES {"capabilities"};
const QString K_TEXTDOCUMENT {"textDocument"};
const QString K_DOCUMENTSYMBOL {"documentSymbol"};
const QString K_HIERARCHICALDOCUMENTSYMBOLSUPPORT {"hierarchicalDocumentSymbolSupport"};
const QString K_PUBLISHDIAGNOSTICS {"publishDiagnostics"};
const QString K_RELATEDINFOMATION {"relatedInformation"};
const QString K_INITIALIZATIONOPTIONS {"initializationOptions"};
const QString K_PROCESSID {"processId"};
const QString K_ROOTPATH {"rootPath"};
const QString K_ROOTURI {"rootUri"};
const QString K_URI {"uri"}; // value QString from file url
const QString K_VERSION {"version"}; // value int
const QString K_LANGUAGEID {"languageId"};
const QString K_TEXT {"text"};
const QString K_CONTAINERNAME {"containerName"};
const QString K_KIND {"kind"};
const QString K_LOCATION {"location"};
const QString K_POSITION {"position"};
const QString K_DATA{"data"};
const QString K_NewName{"newName"};
const QString K_NewText{"newText"};

const QString H_CONTENT_LENGTH {"Content-Length"};
const QString V_2_0 {"2.0"};
const QString V_INITIALIZE {"initialize"}; //has request result
const QString V_SHUTDOWN {"shutdown"}; //has request result
const QString V_EXIT{"exit"}; //has request result
const QString V_TEXTDOCUMENT_DIDOPEN {"textDocument/didOpen"}; //no request result
const QString V_TEXTDOCUMENT_PUBLISHDIAGNOSTICS {"textDocument/publishDiagnostics"}; //server call
const QString V_TEXTDOCUMENT_DIDCHANGE {"textDocument/didChange"}; //no request result, json error
const QString V_TEXTDOCUMENT_DOCUMENTSYMBOL {"textDocument/documentSymbol"}; // has request result
const QString V_TEXTDOCUMENT_HOVER {"textDocument/hover"}; // has request result
const QString V_TEXTDOCUMENT_RENAME {"textDocument/rename"}; // has request result
const QString V_TEXTDOCUMENT_DEFINITION {"textDocument/definition"};
const QString V_TEXTDOCUMENT_DIDCLOSE {"textDocument/didClose"};
const QString V_TEXTDOCUMENT_COMPLETION {"textDocument/completion"};
const QString V_TEXTDOCUMENT_SIGNATUREHELP {"textDocument/signatureHelp"};
const QString V_TEXTDOCUMENT_REFERENCES {"textDocument/references"};
const QString V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT {"textDocument/documentHighlight"};
const QString V_TEXTDOCUMENT_SEMANTICTOKENS {"textDocument/semanticTokens"};
const QString V_TEXTDOCUMENT_SEMANTICTOKENS_FULL{"textDocument/semanticTokens/full"};
const QString K_WORKSPACEFOLDERS {"workspaceFolders"};

const QString K_CONTENTCHANGES {"contentChanges"};
const QString K_DIAGNOSTICS {"diagnostics"};
const QString K_RANGE {"range"};
const QString K_MESSAGE {"message"};
const QString K_SEVERITY {"severity"};
const QString K_END {"end"};
const QString K_START {"start"};
const QString K_CHARACTER {"character"};
const QString K_LINE {"line"};
const QString K_CONTEXT {"context"};
const QString K_INCLUDEDECLARATION {"includeDeclaration"};

const QString K_ERROR {"error"};
const QString K_CODE {"code"};

QList<int> fromTokenModifiers(int modifiers)
{
    QList<int> ret;
    int temp = modifiers;
    ret.push_front((int)(temp %10));
    while (temp / 10 >= 1) {
        temp = temp / 10;
        ret.push_front((int)(temp %10));
    }

    return ret;
}

QJsonArray tokenTypes()
{
    return {
        "namespace",
        "type",
        "class",
        "enum",
        "interface",
        "struct",
        "typeParameter",
        "parameter",
        "variable",
        "property",
        "enumMember",
        "event",
        "function",
        "method",
        "macro",
        "keyword",
        "modifier",
        "comment",
        "string",
        "number",
        "regexp",
        "operator"
    };
}

QJsonArray tokenModifiers()
{
    return {
        "declaration",
        "definition",
        "readonly",
        "static",
        "deprecated",
        "abstract",
        "async",
        "modification",
        "documentation",
        "defaultLibrary"
    };
}

QJsonObject workspace()
{
    QJsonObject didChangeConfiguration {
        { "dynamicRegistration", true }
    };
    QJsonObject codeLens {
        { "refreshSupport", true }
    };
    QJsonObject didChangeWatchedFiles {
        { "dynamicRegistration", true}
    };
    QJsonObject executeCommand{
        { "dynamicRegistration", true}
    };
    QJsonObject fileOperations {
        { "didCreate", true},
        { "didDelete", true},
        { "didRename", true},
        { "dynamicRegistration", true},
        { "willCreate", true},
        { "willDelete", true},
        { "willRename", true}
    };
    QJsonObject semanticTokens{
        { "refreshSupport", true }
    };

    QJsonArray symbolKind_valueSet { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26 };
    QJsonObject symbolKind{ { "valueSet", symbolKind_valueSet } };
    QJsonArray tagSupport_valueSet{1};
    QJsonObject tagSupport{ { "valueSet", tagSupport_valueSet } };
    QJsonObject symbol{
        { "dynamicRegistration", true},
        { "symbolKind", symbolKind },
        { "tagSupport", tagSupport }
    };

    QJsonArray resourceOperations{
        { "create", "rename", "delete" }
    };

    QJsonObject changeAnnotationSupport{
        { "groupsOnLabel", true }
    };
    QJsonObject workspaceEdit{
        { "changeAnnotationSupport",changeAnnotationSupport },
        { "documentChanges", true },
        { "failureHandling", "textOnlyTransactional" },
        { "normalizesLineEndings", true },
        { "resourceOperations", resourceOperations}
    };

    QJsonObject workspace {
        { "applyEdit", true },
        { "codeLens", codeLens },
        { "configuration", true },
        { "didChangeConfiguration", didChangeConfiguration },
        { "didChangeWatchedFiles", didChangeWatchedFiles },
        { "executeCommand", executeCommand },
        { "fileOperations", fileOperations },
        { "semanticTokens", semanticTokens },
        { "symbol",symbol },
        { "workspaceEdit", workspaceEdit },
        { "workspaceFolders", true }
    };

    return workspace;
}

QJsonObject initialize(const QString &workspaceFolder, const QString &language, const QString &compile)
{
    QJsonObject params;
    if (language == C_CPP) {
        QJsonObject capabilitiesSemanticTokens{{
                {"dynamicRegistration", true},
                {"formats", QJsonArray{"relative"}},
                {"multilineTokenSupport", false},
                {"overlappingTokenSupport", false},
                {"requests", QJsonArray{"full",QJsonObject{{"delta", true}},QJsonObject{{"range", true}}}},
                {"tokenModifiers", tokenModifiers()},
                {"tokenTypes", tokenTypes()}}};

        QJsonObject capabilities {
            {
                K_TEXTDOCUMENT, QJsonObject {
                    {"documentLink", QJsonObject{{"dynamicRegistration", true}}},
                    {"documentHighlight", QJsonObject{{"dynamicRegistration", true}}},
                    {K_DOCUMENTSYMBOL, QJsonObject{{K_HIERARCHICALDOCUMENTSYMBOLSUPPORT, true}},},
                    {K_PUBLISHDIAGNOSTICS, QJsonObject{{K_RELATEDINFOMATION, true}}},
                    {"definition", QJsonObject{{{"dynamicRegistration", true},{"linkSupport", true}}}},
                    {"colorProvider", QJsonObject{{"dynamicRegistration", true}}},
                    {"declaration", QJsonObject{{"dynamicRegistration", true},{"linkSupport", true}}},
                    {"semanticHighlightingCapabilities", QJsonObject{{"semanticHighlighting", true}}},
                    {"semanticTokens", capabilitiesSemanticTokens}
                }
            },{
                "workspace", workspace()
            },{
                "foldingRangeProvider", true,
            }
        };

        QJsonObject highlight {
            {"largeFileSize", 2097152},
            {"lsRanges", false},
            {"blacklist", QJsonArray()},
            {"whitelist", QJsonArray()}
        };

        QJsonObject client{
            {"diagnosticsRelatedInformation", true},
            {"hierarchicalDocumentSymbolSupport", true},
            {"linkSupport",true},
            {"snippetSupport",true},
        };

        QJsonObject workspace {
            { "name", QFileInfo(workspaceFolder).fileName() },
            { K_URI, QUrl::fromLocalFile(workspaceFolder).toString() }
        };

        QJsonArray workspaceFolders{workspace};

        QJsonObject initializationOptions {
            {"compilationDatabasePath", compile}
        };

        QJsonObject params {
            { K_PROCESSID, QCoreApplication::applicationPid() },
            { K_ROOTPATH, workspace },
            { K_ROOTURI, workspace },
            { K_CAPABILITIES, capabilities },
            { "highlight", highlight },
            { "client", client },
            { "initializationOptions", initializationOptions},
            { "workspaceFolders", workspaceFolders },
        };

        return params;

    } else if (language == JAVA || language == PYTHON || language == JS) {

        using namespace newlsp;

        std::string workspaceFolderUriStr = QUrl::fromLocalFile(workspaceFolder).toString().toStdString();
        std::string workspaceNameStr = QFileInfo(workspaceFolder).fileName().toStdString();

        InitializeParams params;

        /* processId */
        params.processId = QCoreApplication::applicationPid();

        /* rootUri */
        params.rootUri = workspaceFolderUriStr;

        /* rootPath */
        params.rootPath = workspaceFolder.toStdString();

        /* capabilities
         * std::optional<Workspace> workspace{};
         * std::optional<TextDocumentClientCapabilities> textDocument{};
         * std::optional<NotebookDocumentClientCapabilities> notebookDocument{};
         * std::optional<Window> window{};
         * std::optional<General> general{};
         */
        {
            /* workspace */
            {
                params.capabilities.workspace = newlsp::Workspace{};
                params.capabilities.workspace->applyEdit = true;
                /* WorkspaceEditClientCapabilities */
                params.capabilities.workspace->workspaceEdit = newlsp::WorkspaceEditClientCapabilities{};
                params.capabilities.workspace->workspaceEdit->documentChanges = true;
                params.capabilities.workspace->workspaceEdit->resourceOperations = newlsp::Enum::ResourceOperationKind::toStdVector();
                params.capabilities.workspace->workspaceEdit->failureHandling = newlsp::Enum::FailureHandlingKind::get()->TextOnlyTransactional;
                params.capabilities.workspace->workspaceEdit->normalizesLineEndings = true;
                params.capabilities.workspace->workspaceEdit->changeAnnotationSupport = ChangeAnotationSupport{};
                params.capabilities.workspace->workspaceEdit->changeAnnotationSupport->groupsOnLabel = true;

                /* DidChangeConfigurationClientCapabilities */
                params.capabilities.workspace->didChangeConfiguration = DidChangeConfigurationClientCapabilities{};
                params.capabilities.workspace->didChangeConfiguration->dynamicRegistration = true;

                /* DidChangeWatchedFilesClientCapabilities */
                params.capabilities.workspace->didChangeWatchedFiles = DidChangeWatchedFilesClientCapabilities{};
                params.capabilities.workspace->didChangeWatchedFiles->dynamicRegistration = true;
                // no setting from vscode
                // params.capabilities.workspace->didChangeWatchedFiles->relativePatternSupport = true;

                /* WorkspaceSymbolClientCapabilities*/
                params.capabilities.workspace->symbol = WorkspaceSymbolClientCapabilities{};
                params.capabilities.workspace->symbol->dynamicRegistration = true;
                params.capabilities.workspace->symbol->symbolKind = SymbolKind{};
                params.capabilities.workspace->symbol->symbolKind->valueSet = newlsp::Enum::SymbolKind::toStdVector();
                params.capabilities.workspace->symbol->tagSupport = WorkspaceSymbolClientCapabilities::TagSupport{};
                params.capabilities.workspace->symbol->tagSupport->valueSet = newlsp::Enum::SymbolTag::toStdVector();
                // no setting from vscode
                // params.capabilities.workspace->symbol->resolveSupport = {};

                /* ExecuteCommandClientCapabilities */
                params.capabilities.workspace->executeCommand = ExecuteCommandClientCapabilities{};
                params.capabilities.workspace->executeCommand->dynamicRegistration = true;

                /* workspaceFolders */
                params.capabilities.workspace->workspaceFolders = true;

                /* configuration */
                params.capabilities.workspace->configuration = true;

                /* SemanticTokensWorkspaceClientCapabilities */
                params.capabilities.workspace->semanticTokens = SemanticTokensWorkspaceClientCapabilities{};
                params.capabilities.workspace->semanticTokens->refreshSupport = true;

                /* CodeLensWorkspaceClientCapabilities */
                params.capabilities.workspace->codeLens = CodeLensWorkspaceClientCapabilities{};
                params.capabilities.workspace->codeLens->refreshSupport = true;

                /* FileOperations */
                params.capabilities.workspace->fileOperations = FileOperations{};
                params.capabilities.workspace->fileOperations->dynamicRegistration = true;
                params.capabilities.workspace->fileOperations->didCreate = true;
                params.capabilities.workspace->fileOperations->willCreate = true;
                params.capabilities.workspace->fileOperations->didRename = true;
                params.capabilities.workspace->fileOperations->willRename = true;
                params.capabilities.workspace->fileOperations->didDelete = true;
                params.capabilities.workspace->fileOperations->willDelete = true;

                /* InlineValueWorkspaceClientCapabilities */
                //no setting from vscode
                // params.capabilities.workspace->inlineValue = InlineValueWorkspaceClientCapabilities{};
                // params.capabilities.workspace->inlineValue->refreshSupport = true;

                /* InlayHintWorkspaceClientCapabilities */
                // no setting from vscode
                // params.capabilities.workspace->inlayHint = InlayHintWorkspaceClientCapabilities{};
                // params.capabilities.workspace->inlayHint->refreshSupport = true;

                /* DiagnosticWorkspaceClientCapabilities */
                // no setting from vscode
                // params.capabilities.workspace->diagnostics = DiagnosticWorkspaceClientCapabilities{};
                // params.capabilities.workspace->diagnostics->refreshSupport = true;
            }
            /* textDocument */
            {
                params.capabilities.textDocument = TextDocumentClientCapabilities{};

                /* TextDocumentSyncClientCapabilities */
                params.capabilities.textDocument->synchronization = TextDocumentSyncClientCapabilities{};
                params.capabilities.textDocument->synchronization->dynamicRegistration = true;
                params.capabilities.textDocument->synchronization->willSave = true;
                params.capabilities.textDocument->synchronization->willSaveWaitUntil = true;
                params.capabilities.textDocument->synchronization->didSave = true;

                /* CompletionClientCapabilities */
                params.capabilities.textDocument->completion = CompletionClientCapabilities{};
                params.capabilities.textDocument->completion->dynamicRegistration = true;
                params.capabilities.textDocument->completion->contextSupport = true;

                params.capabilities.textDocument->completion->completionItem = CompletionClientCapabilities::CompletionItem{};
                params.capabilities.textDocument->completion->completionItem->snippetSupport = true;
                params.capabilities.textDocument->completion->completionItem->commitCharactersSupport = true;
                params.capabilities.textDocument->completion->completionItem->documentationFormat = newlsp::Enum::MarkupKind::toStdVector();
                params.capabilities.textDocument->completion->completionItem->deprecatedSupport = true;
                params.capabilities.textDocument->completion->completionItem->preselectSupport = true;
                params.capabilities.textDocument->completion->completionItem->tagSupport
                        = CompletionClientCapabilities::CompletionItem::TagSupport{};
                params.capabilities.textDocument->completion->completionItem->tagSupport->valueSet
                        = newlsp::Enum::CompletionItemTag::toStdVector();
                params.capabilities.textDocument->completion->completionItem->insertReplaceSupport = true;
                params.capabilities.textDocument->completion->completionItem->resolveSupport = ResolveSupport{};
                params.capabilities.textDocument->completion->completionItem->resolveSupport->properties = {
                    "documentation",
                    "detail",
                    "additionalTextEdits"
                }; // from vs code
                params.capabilities.textDocument->completion->completionItem->insertTextModeSupport
                        = CompletionClientCapabilities::CompletionItem::InsertTextModeSupport{};
                params.capabilities.textDocument->completion->completionItem->insertTextModeSupport->valueSet
                        = newlsp::Enum::InsertTextMode::toStdVector();
                params.capabilities.textDocument->completion->completionItem->labelDetailsSupport = true;

                /* insertTextMode: 2 */
                params.capabilities.textDocument->completion->insertTextMode
                        = newlsp::Enum::InsertTextMode::get()->adjustIndentation; // from vscode

                /* completionItemKind */
                params.capabilities.textDocument->completion->completionItemKind
                        = CompletionClientCapabilities::CompletionItemKind{};
                params.capabilities.textDocument->completion->completionItemKind->valueSet
                        = newlsp::Enum::CompletionItemKind::toStdVector();

                /* itemDefaults */ // no setting from vscode
                // params.capabilities.textDocument->completion->itemDefaults = {std::nullopt};

                /* contextSupport */
                params.capabilities.textDocument->completion->contextSupport = true;

                /* HoverClientCapabilities */
                params.capabilities.textDocument->hover = HoverClientCapabilities{};
                params.capabilities.textDocument->hover->dynamicRegistration = true;
                params.capabilities.textDocument->hover->contentFormat = newlsp::Enum::MarkupKind::toStdVector();

                /* SignatureHelpClientCapabilities */
                params.capabilities.textDocument->signatureHelp = SignatureHelpClientCapabilities{};
                params.capabilities.textDocument->signatureHelp->dynamicRegistration = true;
                params.capabilities.textDocument->signatureHelp->signatureInformation = SignatureHelpClientCapabilities::SignatureInformation{};
                params.capabilities.textDocument->signatureHelp->signatureInformation->documentationFormat = newlsp::Enum::MarkupKind::toStdVector();
                params.capabilities.textDocument->signatureHelp->signatureInformation->parameterInformation
                        = SignatureHelpClientCapabilities::SignatureInformation::ParameterInformation{};
                params.capabilities.textDocument->signatureHelp->signatureInformation->parameterInformation->labelOffsetSupport = true;
                params.capabilities.textDocument->signatureHelp->signatureInformation->activeParameterSupport = true;
                params.capabilities.textDocument->signatureHelp->contextSupport = true;

                /* DeclarationClientCapabilities */
                params.capabilities.textDocument->declaration = DeclarationClientCapabilities{};
                params.capabilities.textDocument->declaration->dynamicRegistration = true;
                params.capabilities.textDocument->declaration->linkSupport = true;

                /* DefinitionClientCapabilities */
                params.capabilities.textDocument->definition = DefinitionClientCapabilities{};
                params.capabilities.textDocument->definition->dynamicRegistration = true;
                params.capabilities.textDocument->definition->linkSupport = true;

                /* TypeDefinitionClientCapabilities */
                params.capabilities.textDocument->typeDefinition = TypeDefinitionClientCapabilities{};
                params.capabilities.textDocument->typeDefinition->dynamicRegistration = true;
                params.capabilities.textDocument->typeDefinition->linkSupport = true;

                /* ImplementationClientCapabilities */
                params.capabilities.textDocument->implementation = ImplementationClientCapabilities{};
                params.capabilities.textDocument->implementation->dynamicRegistration = true;
                params.capabilities.textDocument->implementation->linkSupport = true;

                /* ReferenceClientCapabilities */
                params.capabilities.textDocument->references = ReferenceClientCapabilities{};
                params.capabilities.textDocument->references->dynamicRegistration = true;

                /* DocumentHighlightClientCapabilities */
                params.capabilities.textDocument->documentHighlight = DocumentHighlightClientCapabilities{};
                params.capabilities.textDocument->documentHighlight->dynamicRegistration = true;

                /* DocumentSymbolClientCapabilities */
                params.capabilities.textDocument->documentSymbol = DocumentSymbolClientCapabilities{};
                params.capabilities.textDocument->documentSymbol->dynamicRegistration = true;
                params.capabilities.textDocument->documentSymbol->symbolKind = SymbolKind{};
                params.capabilities.textDocument->documentSymbol->symbolKind->valueSet = newlsp::Enum::SymbolKind::toStdVector();
                params.capabilities.textDocument->documentSymbol->hierarchicalDocumentSymbolSupport = true;
                params.capabilities.textDocument->documentSymbol->tagSupport = DocumentSymbolClientCapabilities::TagSupport{};
                params.capabilities.textDocument->documentSymbol->tagSupport->valueSet = newlsp::Enum::SymbolTag::toStdVector();
                params.capabilities.textDocument->documentSymbol->labelSupport = true;

                /* CodeActionClientCapabilities */ //func
                params.capabilities.textDocument->codeAction = CodeActionClientCapabilities{};
                params.capabilities.textDocument->codeAction->dynamicRegistration = true;
                params.capabilities.textDocument->codeAction->isPreferredSupport = true;
                params.capabilities.textDocument->codeAction->disabledSupport = true;
                params.capabilities.textDocument->codeAction->dataSupport = true;
                params.capabilities.textDocument->codeAction->resolveSupport = ResolveSupport{};
                params.capabilities.textDocument->codeAction->resolveSupport->properties = {newlsp::Enum::Properties::get()->edit};
                params.capabilities.textDocument->codeAction->codeActionLiteralSupport = CodeActionLiteralSupport{};
                params.capabilities.textDocument->codeAction->codeActionLiteralSupport->codeActionKind = CodeActionKind{};
                params.capabilities.textDocument->codeAction->codeActionLiteralSupport->codeActionKind.valueSet
                        = newlsp::Enum::CodeActionKind::toStdVector();
                params.capabilities.textDocument->codeAction->honorsChangeAnnotations = false;

                /* CodeLensClientCapabilities */
                params.capabilities.textDocument->codeLens = CodeLensClientCapabilities{};
                params.capabilities.textDocument->codeLens->dynamicRegistration = true;

                /* DocumentLinkClientCapabilities */
                params.capabilities.textDocument->documentLink = DocumentLinkClientCapabilities {};
                params.capabilities.textDocument->documentLink->dynamicRegistration = true;
                params.capabilities.textDocument->documentLink->tooltipSupport = true;

                /* DocumentColorClientCapabilities */
                params.capabilities.textDocument->colorProvider = DocumentColorClientCapabilities{};
                params.capabilities.textDocument->colorProvider->dynamicRegistration = true;

                /* DocumentFormattingClientCapabilities */
                params.capabilities.textDocument->formatting = DocumentFormattingClientCapabilities{};
                params.capabilities.textDocument->formatting->dynamicRegistration = true;

                /* DocumentFormattingClientCapabilities */
                params.capabilities.textDocument->rangeFormatting = DocumentRangeFormattingClientCapabilities{};
                params.capabilities.textDocument->rangeFormatting->dynamicRegistration = true;

                /* DocumentRangeFormattingClientCapabilities */
                params.capabilities.textDocument->onTypeFormatting = DocumentOnTypeFormattingClientCapabilities{};
                params.capabilities.textDocument->onTypeFormatting->dynamicRegistration = true;

                /* RenameClientCapabilities */
                params.capabilities.textDocument->rename = RenameClientCapabilities{};
                params.capabilities.textDocument->rename->dynamicRegistration = true;
                params.capabilities.textDocument->rename->prepareSupport = true;
                params.capabilities.textDocument->rename->prepareSupportDefaultBehavior
                        = newlsp::Enum::PrepareSupportDefaultBehavior::get()->Identifier;
                params.capabilities.textDocument->rename->honorsChangeAnnotations = true;

                /* PublishDiagnosticsClientCapabilities */
                params.capabilities.textDocument->publishDiagnostics = PublishDiagnosticsClientCapabilities{};
                params.capabilities.textDocument->publishDiagnostics->relatedInformation = true;
                params.capabilities.textDocument->publishDiagnostics->tagSupport
                        = PublishDiagnosticsClientCapabilities::TagSupport{};
                params.capabilities.textDocument->publishDiagnostics->tagSupport->valueSet = newlsp::Enum::DiagnosticTag::toStdVector();
                params.capabilities.textDocument->publishDiagnostics->versionSupport = false;
                params.capabilities.textDocument->publishDiagnostics->codeDescriptionSupport = true;
                params.capabilities.textDocument->publishDiagnostics->dataSupport = true;

                /* FoldingRangeClientCapabilities */
                params.capabilities.textDocument->foldingRange = FoldingRangeClientCapabilities{};
                params.capabilities.textDocument->foldingRange->dynamicRegistration = true;
                params.capabilities.textDocument->foldingRange->rangeLimit = 5000; // vscode 5000
                params.capabilities.textDocument->foldingRange->lineFoldingOnly = true;
                // no setting from vscode
                // params.capabilities.textDocument->foldingRange->foldingRangeKind = FoldingRangeKind{};
                // params.capabilities.textDocument->foldingRange->foldingRangeKind->valueSet = {newlsp::Enum::FoldingRangeKind::toStdVector()};
                // params.capabilities.textDocument->foldingRange->foldingRange = FoldingRange{};
                // params.capabilities.textDocument->foldingRange->foldingRange->collapsedText = true;

                /* SelectionRangeClientCapabilities */
                params.capabilities.textDocument->selectionRange = SelectionRangeClientCapabilities{};
                params.capabilities.textDocument->selectionRange->dynamicRegistration = true;

                /* LinkedEditingRangeClientCapabilities */
                params.capabilities.textDocument->linkedEditingRange = LinkedEditingRangeClientCapabilities{};
                params.capabilities.textDocument->linkedEditingRange->dynamicRegistration = true;

                /* CallHierarchyClientCapabilities */
                params.capabilities.textDocument->callHierarchy = CallHierarchyClientCapabilities{};
                params.capabilities.textDocument->callHierarchy->dynamicRegistration = true;

                /* SemanticTokensClientCapabilities */
                params.capabilities.textDocument->semanticTokens = SemanticTokensClientCapabilities{};
                params.capabilities.textDocument->semanticTokens->dynamicRegistration = true;
                params.capabilities.textDocument->semanticTokens->tokenTypes = newlsp::Enum::SemanticTokenTypes::toStdVector();
                params.capabilities.textDocument->semanticTokens->tokenModifiers = newlsp::Enum::SemanticTokenModifiers::toStdVector();
                params.capabilities.textDocument->semanticTokens->formats = newlsp::Enum::TokenFormat::toStdVector();
                params.capabilities.textDocument->semanticTokens->requests = SemanticTokensClientCapabilities::Requests{};
                params.capabilities.textDocument->semanticTokens->requests.full = SemanticTokensClientCapabilities::Requests::Full{};
                params.capabilities.textDocument->semanticTokens->requests.full->delta = true;
                params.capabilities.textDocument->semanticTokens->requests.range = std::optional<std::any>(true);
                params.capabilities.textDocument->semanticTokens->multilineTokenSupport = false; // from vscode
                params.capabilities.textDocument->semanticTokens->overlappingTokenSupport = false; // from vscode
                // no setting from vscode
                // params.capabilities.textDocument->semanticTokens->serverCancelSupport = true;
                // params.capabilities.textDocument->semanticTokens->augmentsSyntaxTokens = true;

                /* MonikerClientCapabilities */ // no setting from vscode
                // MonikerClientCapabilities monikerClientCapabilities{};
                // monikerClientCapabilities.dynamicRegistration = true;
                // params.capabilities.textDocument->moniker = {monikerClientCapabilities};

                /* TypeHierarchyClientCapabilities */ // no setting from vscode
                // TypeHierarchyClientCapabilities typeHierarchyClientCapabilities{};
                // typeHierarchyClientCapabilities.dynamicRegistration = true;
                // params.capabilities.textDocument->typeHierarchy = {typeHierarchyClientCapabilities};

                /* InlineValueClientCapabilities */ // no setting from vscode
                // InlineValueClientCapabilities inlineValueClientCapabilities{};
                // inlineValueClientCapabilities.dynamicRegistration = true;
                // params.capabilities.textDocument->inlineValue = {inlineValueClientCapabilities};

                /* InlayHintClientCapabilities */ // no setting from vscode
                // params.capabilities.textDocument->inlayHint = InlayHintClientCapabilities{};
                // params.capabilities.textDocument->inlayHint->dynamicRegistration = true;
                // params.capabilities.textDocument->inlayHint->resolveSupport = ResolveSupport{};
                // params.capabilities.textDocument->inlayHint->resolveSupport->properties
                //         = {newlsp::Enum::Properties::get()->label_location};

                /* DiagnosticClientCapabilities */ // no setting from vscode
                // DiagnosticClientCapabilities diagnosticClientCapabilities{};
                // diagnosticClientCapabilities.dynamicRegistration = true;
                // diagnosticClientCapabilities.relatedDocumentSupport = true;
                // params.capabilities.textDocument->diagnostic = {diagnosticClientCapabilities};
            }
            /* notebookDocument */
            {
                // no setting from vscode
                // params.capabilities.notebookDocument = NotebookDocumentClientCapabilities{};
                // params.capabilities.notebookDocument->synchronization.dynamicRegistration = true;
                // params.capabilities.notebookDocument->synchronization.executionSummarySupport = true;
            }
            /* window */
            {
                params.capabilities.window = Window{};
                params.capabilities.window->workDoneProgress = true;
                params.capabilities.window->showMessage = ShowMessageRequestClientCapabilities{};
                params.capabilities.window->showMessage->messageActionItem = MessageActionItem{};
                params.capabilities.window->showMessage->messageActionItem->additionalPropertiesSupport = true;
                params.capabilities.window->showDocument = ShowDocumentClientCapabilities{};
                params.capabilities.window->showDocument->support = true;
            }
            /* general */
            {
                params.capabilities.general = General{};
                params.capabilities.general->staleRequestSupport = StaleRequestSupport{};
                params.capabilities.general->staleRequestSupport->cancel = true;
                params.capabilities.general->staleRequestSupport->retryOnContentModified = {
                    "textDocument/semanticTokens/full",
                    "textDocument/semanticTokens/range",
                    "textDocument/semanticTokens/full/delta"
                };
                params.capabilities.general->regularExpressions = RegularExpressionsClientCapabilities{};
                params.capabilities.general->regularExpressions->engine = "ECMAScript";
                params.capabilities.general->regularExpressions->version = "ES2020";
                params.capabilities.general->markdown = MarkdownClientCapabilities{};
                params.capabilities.general->markdown->parser = "marked";
                params.capabilities.general->markdown->version = "1.1.0";
            }
        }

        /* trace */
        {
            params.trace = newlsp::Enum::TraceValue::get()->Verbose;
        }

        /* workspaceFolders */
        {
            std::vector<WorkspaceFolder> workspaceFolders{};
            WorkspaceFolder folderParams;
            folderParams.uri = workspaceFolderUriStr;
            folderParams.name = workspaceNameStr;
            workspaceFolders.push_back(folderParams);
            params.workspaceFolders = {workspaceFolders};
        }

        QJsonParseError err;
        auto jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(toJsonValueStr(params)), &err);
        if (err.error != QJsonParseError::NoError)
            qCritical() << err.errorString();
        return jsonDoc.object();
    }
    return {};
}

QJsonObject didOpen(const QString &filePath)
{
    QFile file(filePath);
    QString text;
    if (!file.open(QFile::ReadOnly)) {
        qCritical()<< "Failed, open file: "
                   << filePath <<file.errorString();
    }
    text = file.readAll();
    file.close();

    QJsonObject textDocument{
        { K_URI, QUrl::fromLocalFile(filePath).toString() },
        { K_LANGUAGEID, "cpp" },
        { K_VERSION, 1 },
        { K_TEXT, text }
    };

    QJsonObject params{
        { K_TEXTDOCUMENT, textDocument }
    };

    return params;
}

// full mode
QJsonObject didChange(const QString &filePath, const QByteArray &text, int version)
{
    QJsonObject changeEvent
    {
        { K_TEXT, QString::fromUtf8(text)}
    };

    QJsonArray contentChanges
    {
        changeEvent
    };

    QJsonObject textDocument
    {
        { K_URI, QUrl::fromLocalFile(filePath).toString() },
        { K_VERSION, version }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument },
        { "contentChanges", contentChanges }
    };

    return params;
}

QJsonObject didClose(const QString &filePath)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject params{
        { K_TEXTDOCUMENT, textDocument},
    };

    return params;
}

QJsonObject hover(const QString &filePath, const Position &pos)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() },
    };

    QJsonObject position {
        { K_CHARACTER, pos.character },
        { K_LINE, pos.line }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument},
        { K_POSITION, position }
    };

    return params;
}

QJsonObject definition(const QString &filePath, const Position &pos)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject position {
        { K_CHARACTER, pos.character },
        { K_LINE, pos.line }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument },
        { K_POSITION, position}
    };

    return params;
}

QJsonObject signatureHelp(const QString &filePath, const Position &pos)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject position {
        { K_CHARACTER, pos.character },
        { K_LINE, pos.line }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument },
        { K_POSITION, position}
    };

    return params;
}

QJsonObject references(const QString &filePath, const Position &pos)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject position {
        { K_CHARACTER, pos.character },
        { K_LINE, pos.line }
    };

    QJsonObject context {
        { K_INCLUDEDECLARATION, true}
    };

    QJsonObject params {
        { K_CONTEXT, context},
        { K_TEXTDOCUMENT, textDocument },
        { K_POSITION, position}
    };

    return params;
}

QJsonObject documentHighlight(const QString &filePath, const Position &pos)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject position {
        { K_CHARACTER, pos.character },
        { K_LINE, pos.line }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument },
        { K_POSITION, position }
    };

    return params;
}

//more see https://microsoft.github.io/language-server-protocol/specifications/specification-current/#textDocument_semanticTokens
QJsonObject documentSemanticTokensFull(const QString &filePath)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument }
    };

    return params;
}

QJsonObject documentSemanticTokensRange(const QString &filePath)
{
    Q_UNUSED(filePath)
    return {};
}

QJsonObject documentSemanticTokensDelta(const QString &filePath)
{
    Q_UNUSED(filePath)
    return {};
}

QJsonObject shutdown()
{
    return {};
}

QJsonObject symbol(const QString &filePath)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument }
    };

    return params;
}

QJsonObject rename(const QString &filePath, const Position &pos, const QString &newName)
{
    QJsonObject params;
    params[K_NewName] = newName;
    params[K_POSITION] = QJsonObject{{K_LINE, pos.line},{K_CHARACTER, pos.character}};
    params[K_TEXTDOCUMENT] = QJsonObject{{K_URI, QUrl::fromLocalFile(filePath).toString()}} ;

    return params;
}

QJsonObject completion(const QString &filePath, const Position &pos)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject position {
        { K_CHARACTER, pos.character },
        { K_LINE, pos.line }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument },
        { K_POSITION, position}
    };

    return params;
}

bool isRequestResult(const QJsonObject &object)
{
    QStringList keys = object.keys();
    if (keys.contains(K_ID) && keys.contains(K_RESULT))
        return true;
    return false;
}

bool isRequestError(const QJsonObject &object)
{
    if (object.keys().contains(K_ERROR)) {
        qInfo() << "Failed, Request error";
        return true;
    }
    return false;
}

QJsonObject exit()
{
    return {};
}

}
