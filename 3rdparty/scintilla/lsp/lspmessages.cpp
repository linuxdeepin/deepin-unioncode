// Scintilla source code edit control
/** @file lspmessages.cpp
 ** Class for defining LSP messages used
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "lspmessages.h"
#include "lspmessagehandler.h"
#include "lspprocess.h"
#include "lspstructures.h"

#include "../include/Platform.h"

using namespace Scintilla;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static lspMessage markupKind(LspClientConfiguration::MarkupKind kind)
{
	lspMessage ret;
	switch (kind)
	{
		case LspClientConfiguration::Markdown:
			ret.emplace_back(LspClientConfiguration::toString(kind));
			break;
		case LspClientConfiguration::PlainText:
			break;
	}
	// Plain text always supported
	ret.emplace_back(LspClientConfiguration::toString(LspClientConfiguration::PlainText));
	return ret;
}

//! \brief Generate message for configuration
static lspMessage getConfiguration(const LspClientConfiguration &cfg)
{
	//
	//	Configuration of textDocument capabilities
	//
	lspMessage textDocument;
	if (cfg.semanticHighlighting.supported)
		textDocument["semanticHighlightingCapabilities"]["semanticHighlighting"] = true;
	if (cfg.publishDiagnostics.supported)
	{
		textDocument["publishDiagnostics"]["relatedInformation"] = cfg.publishDiagnostics.relatedInformation;
		if (cfg.publishDiagnostics.categorySupport)
			textDocument["publishDiagnostics"]["categorySupport"] = true;
		if (cfg.publishDiagnostics.codeActionsInline)
			textDocument["publishDiagnostics"]["codeActionsInline"] = true;
	}
	if (cfg.documentSymbol.supported)
		textDocument["documentSymbol"]["hierarchicalDocumentSymbolSupport"] = cfg.documentSymbol.hierarchicalDocumentSymbolSupport;
	if (cfg.hover.supported)
		textDocument["hover"]["contentFormat"] = markupKind(cfg.hover.contentFormat);
	if (cfg.signatureHelp.supported)
	{
		textDocument["signatureHelp"] =
			lspMessage({"signatureInformation", {
				{ "documentationFormat", markupKind(cfg.signatureHelp.signatureInformation.documentationFormat) },
				{ "parameterInformation", { { "labelOffsetSupport", cfg.signatureHelp.signatureInformation.parameterInformation.labelOffsetSupport } } },
				},
			});
	}
	//
	//	Configuration of worspace capabilities
	//
	lspMessage workspace;
	if (cfg.didChangeWatchedFiles.supported)
		workspace["didChangeWatchedFiles"] = true;
	// workspace["applyEdit"] = false;	// TODO: support workspace/applyEdit request
	if (cfg.symbol.supported)
		workspace["symbol"] = true;
	if (cfg.executeCommand.supported)
		workspace["executeCommand"] = true;
	if (cfg.configuration.supported)
		workspace["configuration"] = true;
	if (cfg.workspaceFolders.supported)
		workspace["workspaceFolders"] = true;
	//
	//	Composition of the configuration message
	//
	lspMessage config({
		{"processId", Process::GetCurrentProcessId()},
		{"rootPath", cfg.rootUri},
		{"capabilities", {
			{"textDocument", textDocument},
			{"workspace", workspace}
		}}});
	//
	//	If required, add initialization options
	//
	if (cfg.clangdFileStatus)
		config["initializationOptions"] = lspMessage({ { "clangdFileStatus", true } });
	return config;
}

uintptr_t LspMessages::InitializeRequest::send(LspMessageHandler *handler, const LspClientConfiguration &cfg)
{
	const auto id = handler->nextId();
	const bool sentOk = handler->sendMessage(
		id,
		nullptr,
		lspMessage({
			{ "jsonrpc", "2.0" },
			{ "method", "initialize" },
			{ "id", id },
			{ "params", std::move(getConfiguration(cfg)) },
		}).dump(),
		&handleAnswer);
	return (sentOk ? id : 0);
 }

void LspMessages::InitializeRequest::handleAnswer(uintptr_t id, LspClientLogic *logic, LspDocInfo *info, [[maybe_unused]] const std::string &req, const lspMessage &result)
{
	(void)id;		// unused
	(void)info;		// unused
	Platform::DebugPrintf("Server capabilities:\n%s", result.dump(2).c_str());
	logic->GetServerCapabilities().fromMsg(result);
	logic->GetMessageHandler()->sendNotification(lspMessage({
		{ "jsonrpc", "2.0" },
		{ "method", "initialized" }
		}));
	logic->setInitDone();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

uintptr_t LspMessages::DocumentSymbolsRequest::send(LspMessageHandler *handler, LspDocInfo *info)
{
	const auto id = handler->nextId();
	const bool sentOk = handler->sendMessage(
		id,
		info,
		lspMessage({
			{ "jsonrpc", "2.0" },
			{ "method", "textDocument/documentSymbol" },
			{ "id", id },
			{ "params", {
				{ "textDocument", {
					{ "uri", info->uri },
					{ "version", info->version },
				} },
				},
			},
		}).dump(),
		&handleAnswer);
	return (sentOk ? id : 0);
}

void LspMessages::DocumentSymbolsRequest::handleAnswer(uintptr_t id, LspClientLogic *logic, LspDocInfo *info, [[maybe_unused]] const std::string &req, const lspMessage &result)
{
	(void)info;		// unused
	logic->notifyDocumentSymbols(id, LspDocumentSymbols(result));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

uintptr_t LspMessages::DocumentCompletionRequest::send(LspMessageHandler *handler, LspDocInfo *info, const LspPosition &pos)
{
	const auto id = handler->nextId();
	const bool sentOk = handler->sendMessage(
		id,
		info,
		lspMessage({
			{ "jsonrpc", "2.0" },
			{ "method", "textDocument/completion" },
			{ "id", id },
			{ "params", {
				{ "textDocument", {
					{ "uri", info->uri },
					{ "version", info->version },
				} },
				{ "position", pos.toMsg() },
				},
			},
			}).dump(),
			&handleAnswer);
	return (sentOk ? id : 0);
}

void LspMessages::DocumentCompletionRequest::handleAnswer(uintptr_t id, LspClientLogic *logic, LspDocInfo *info, [[maybe_unused]] const std::string &req, const lspMessage &result)
{
	(void)id;		// unused
	(void)info;		// unused
	(void)logic;	// unused
	logic->notifyLspCompletion(id, LspCompletionList(result));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

uintptr_t LspMessages::DocumentSignatureHelpRequest::send(LspMessageHandler *handler, LspDocInfo *info, const LspPosition &pos)
{
	const auto id = handler->nextId();
	const bool sentOk = handler->sendMessage(
		id,
		info,
		lspMessage({
			{ "jsonrpc", "2.0" },
			{ "method", "textDocument/signatureHelp" },
			{ "id", id },
			{ "params", {
				{ "textDocument", {
					{ "uri", info->uri },
					{ "version", info->version },
				} },
				{ "position", pos.toMsg() },
				},
			},
			}).dump(),
			&handleAnswer);
	return (sentOk ? id : 0);
}

void LspMessages::DocumentSignatureHelpRequest::handleAnswer(uintptr_t id, LspClientLogic *logic, LspDocInfo *info, [[maybe_unused]] const std::string &req, const lspMessage &result)
{
	(void)logic;	// unused
	(void)info;		// unused
	LspSignatureHelp help;
	help.fromMsg(result);
	logic->notifyLspSignatureHelp(id, help);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

uintptr_t LspMessages::DocumentHoverRequest::send(LspMessageHandler *handler, LspDocInfo *info, const LspPosition &pos)
{
	const auto id = handler->nextId();
	const bool sentOk = handler->sendMessage(
		id,
		info,
		lspMessage({
			{ "jsonrpc", "2.0" },
			{ "method", "textDocument/hover" },
			{ "id", id },
			{ "params", {
				{ "textDocument", {
					{ "uri", info->uri },
					{ "version", info->version },
				} },
				{ "position", pos.toMsg() },
				},
			},
			}).dump(),
			&handleAnswer);
	return (sentOk ? id : 0);
}

void LspMessages::DocumentHoverRequest::handleAnswer(uintptr_t id, LspClientLogic *logic, LspDocInfo *info, [[maybe_unused]] const std::string &req, const lspMessage &result)
{
	(void)logic;	// unused
	(void)info;		// unused
	LspHover hover;
	hover.fromMsg(result);
	logic->notifyLspHover(id, hover);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspMessages::ShutdownRequest::send(LspMessageHandler *handler)
{
	const auto id = handler->nextId();
	return handler->sendMessage(
		id,
		nullptr,
		lspMessage({
			{ "jsonrpc", "2.0" },
			{ "method", "shutdown" },
			{ "id", id },
			}).dump(),
		&handleAnswer);
}

void LspMessages::ShutdownRequest::handleAnswer(uintptr_t id, LspClientLogic *logic, LspDocInfo *info, [[maybe_unused]] const std::string &req, const lspMessage &result)
{
	(void)id;		// unused
	(void)info;		// unused
	(void)result;	// unused
	LspClientNotifications::ExitNotification::send(logic->GetMessageHandler());
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspClientNotifications::DidOpenTextDocument::send(LspMessageHandler *handler, const LspDocInfo &info)
{
	const auto len = info.doc.Length();
	std::string text;
	text.resize(std::size_t(len));
	info.doc.GetCharRange(text.data(), 0, len);
	lspMessage msg({
		{ "jsonrpc", "2.0" },
		{ "method", "textDocument/didOpen" },
		{ "params", {
			{ "textDocument", {
				{ "uri", info.uri },
				{ "languageId", info.languageId },
				{ "version", info.version },
				{ "text", text } }
				},
			},
		},
	});
	return handler->sendNotification(msg.dump());
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspClientNotifications::DidCloseTextDocument::send(LspMessageHandler *handler, const LspDocInfo &info)
{
	lspMessage msg({
		{ "jsonrpc", "2.0" },
		{ "method", "textDocument/didClose" },
		{ "params", {
			{ "textDocument", { { "uri", info.uri } } }
			},
		},
	});
	return handler->sendNotification(msg.dump());
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspClientNotifications::DidChangeTextDocument::send(LspMessageHandler *handler, LspDocInfo &info, const SCNotification &scn)
{
	if (scn.modificationType & SC_MOD_INSERTTEXT)
	{
		// Platform::DebugPrintf("SC_MOD_INSERTTEXT - line %lld position %lld length %lld text '%.*s', lines added %lld\r\n", scn.line, scn.position, scn.length, int(scn.length), scn.text, scn.linesAdded);
		// Update server status
		// TODO: Needs conversion to utf-8 ?
		std::string text(scn.text, std::size_t(scn.length));
		return handler->sendNotification(lspMessage({
			{ "jsonrpc", "2.0" },
			{ "method", "textDocument/didChange" },
			{ "params", {
				{ "textDocument", {
					{ "uri", info.uri },
					{ "version", ++info.version },
					} },
				{ "contentChanges", nlohmann::json::array({
					{ { "range", lspConv::Range(info.doc, scn.position, 0) }, { "text", text } }
					}) },
				} },
			}).dump());
	}
	if (scn.modificationType & SC_MOD_BEFOREDELETE)
	{
		// Platform::DebugPrintf("SC_MOD_BEFOREDELETE - line %lld position %lld length %lld\r\n", scn.line, scn.position, scn.length);
		// Update server status
		return handler->sendNotification(lspMessage({
			{ "jsonrpc", "2.0" },
			{ "method", "textDocument/didChange" },
			{ "params", {
				{ "textDocument", {
					{ "uri", info.uri },
					{ "version", ++info.version },
					} },
				{ "contentChanges", nlohmann::json::array({
					{ { "range", lspConv::Range(info.doc, scn.position, scn.length) }, { "text", "" } }
					}) },
				} },
			}).dump());
	}
	return false;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspClientNotifications::ExitNotification::send(LspMessageHandler *handler)
{
	return handler->sendNotification(lspMessage({
		{ "jsonrpc", "2.0" },
		{ "method", "exit" }
	}));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	LspServerNotifications
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void LspServerNotifications::PublishDiagnostics::handle(LspClientLogic *logic, const lspMessage &params)
{
	const auto &it_uri = params.find("uri");
	const auto &it_diagnostics = params.find("diagnostics");
	if (it_uri == params.end() || it_diagnostics == params.end())
		return;
	const std::string uri = it_uri->get<std::string>();
	const LspScintillaDoc doc = logic->docFromUri(uri);
	if (doc.isNull())
		return;
	const auto &diagno = params.find("diagnostics");
	LspDocumentDiagnostic dd;
	if (!dd.fromMsg(diagno.value()))
		dd.clear();
	//
	logic->notifyLspDiagnostic(doc, dd);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void LspServerNotifications::SemanticHighlighting::handle(LspClientLogic *logic, const lspMessage &params)
{
	Platform::DebugPrintf("Semantic highlighting:\n%s", params.dump(2).c_str());
	LspSemanticHighlighting sh;
	if (!sh.fromMsg(params))
		return;
	const LspScintillaDoc doc = logic->docFromUri(sh.identifier.uri);
	if (doc.isNull())
		return;
	LspDocumentSemanticHighlighting *const dsh = logic->GetDocumentSemanticHighlighting(doc);
	if (dsh == nullptr)
		return;
	dsh->update(std::move(sh));
	logic->notifyUpdateSemanticHighlighting(doc, *dsh);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void LspServerNotifications::ClangdFileStatus::handle(LspClientLogic *logic, const lspMessage &params)
{
	LspClangdFileStatus sts;
	if (!sts.fromMsg(params))
		return;
	const LspScintillaDoc doc = logic->docFromUri(sts.uri);
	if (doc.isNull())
		return;
	logic->notifyLspFileStatus(doc, sts);
}
