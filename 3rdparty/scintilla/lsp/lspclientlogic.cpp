// Scintilla source code edit control
/** @file lspclientlogic.cpp
 ** Handles the logic of a client communicating with a Language Server
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "lspclientlogic.h"
#include "lspclient.h"
#include "lspmessages.h"
#include "lspmessagehandler.h"
#include "lspstructures.h"
#include "ILspDocument.h"
#include "../include/Platform.h"

using namespace Scintilla;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LspDocInfo::LspDocInfo(const LspScintillaDoc &d, const std::string &_uri, const std::string &_langId)
	: doc(d),
	didOpen(false),
	refCount(1),
	uri(_uri),
	languageId(_langId),
	version(0),
	dsh(nullptr),
	applyingSemanticHighlighting(false),
	semHiglFirstLine(std::numeric_limits<decltype(semHiglFirstLine)>::max()),
	semHiglLastLine(0)
{
}

LspDocInfo::~LspDocInfo()
{
	delete dsh;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LspClientLogic::LspClientLogic(LspClient *client)
	: client(client),
	messageHandler(new LspMessageHandler(this)),
	serverCapabilities(new LspServerCapabilities),
	initDone(false)
{
}

LspClientLogic::~LspClientLogic()
{
	documents.clear();
	// Delete messageHandler, owned by this class
	delete messageHandler;
	// Delete struct
	delete serverCapabilities;
}

bool LspClientLogic::startServer(const char *exec, const char *params)
{
	return messageHandler->startServer(exec, params);
}

bool LspClientLogic::serverStarted() const
{
	return messageHandler->isServerStarted();
}

void LspClientLogic::onServerStarted()
{
	client->onServerStarted();
}

void LspClientLogic::stopServer()
{
	LspMessages::ShutdownRequest::send(messageHandler);
}

bool LspClientLogic::initialize(const LspClientConfiguration &cfg)
{
	return LspMessages::InitializeRequest::send(
		messageHandler,
		cfg);
}

void LspClientLogic::setInitDone()
{
	initDone = true;
	for (auto &it : documents)
		lspAddDocument(it.second);
}

void LspClientLogic::addDocument(const LspScintillaDoc &doc, const std::string &uri, const std::string &languageId)
{
	const auto &it = documents.find(doc);
	if (it == documents.end())
	{
		// New document
		LspDocInfo &info = documents.emplace(std::piecewise_construct,
			std::forward_as_tuple(doc),
			std::forward_as_tuple(doc, uri, languageId)).first->second;
		lspAddDocument(info);
	}
	else
	{
		// Same document having different languages is not supported now.
		// Should give a warning if the same doc is assigned a different language id ?
		it->second.refCount++;
	}
}

void LspClientLogic::onDocumentNotification(const LspScintillaDoc &doc, const SCNotification &scn)
{
	const auto &it = documents.find(doc);
	if (it == documents.end())
		return;
	LspDocInfo &d_info = it->second;
	if (!d_info.didOpen)
		return;
	switch (scn.nmhdr.code)
	{
		case SCN_MODIFIED:
			LspClientNotifications::DidChangeTextDocument::send(messageHandler, d_info, scn);
			if ((scn.modificationType & SC_MOD_CHANGESTYLE) && !d_info.applyingSemanticHighlighting)
			{
//				// Style changed by a lexer: apply semantic highlighting to styled text
				const auto l_from = d_info.doc.LineFromPosition(scn.position);
				const auto l_to = d_info.doc.LineFromPosition(scn.position + scn.length);
				d_info.semHiglFirstLine = std::min(d_info.semHiglFirstLine, l_from);
				d_info.semHiglLastLine = std::max(d_info.semHiglLastLine, l_to);
#ifdef DEBUG
				Platform::DebugPrintf("SC_MOD_LEXERSTATE (from line %d to %d): semantic highlight from line %d to line %d",
					l_from + 1,
					l_to + 1,
					d_info.semHiglFirstLine + 1,
					d_info.semHiglLastLine + 1);
#endif // DEBUG
//				applySemanticHighlighting(d_info);
			}
			break;

		case SCN_UPDATEUI:
			if (scn.updated & SC_UPDATE_CONTENT)
			{
				// Style changed by a lexer: apply semantic highlighting to styled text
//				d_info.semHiglFirstLine = 0;
//				d_info.semHiglLastLine = doc.LineFromPosition(doc.Length());
#ifdef DEBUG
				Platform::DebugPrintf("SCN_UPDATEUI: from line %d to line %d", d_info.semHiglFirstLine + 1, d_info.semHiglLastLine + 1);
#endif
				applySemanticHighlighting(d_info);
			}
			break;
	}
}

void LspClientLogic::notifyLspDiagnostic(const LspScintillaDoc &doc, const LspDocumentDiagnostic &dd)
{
	if (client->notifier)
		client->notifier->notifyLspDiagnostic(doc, dd);
}

void LspClientLogic::notifyUpdateSemanticHighlighting(const LspScintillaDoc &doc, const LspDocumentSemanticHighlighting &dsh)
{
	const auto &it = documents.find(doc);
	if (it == documents.end())
		return;
	LspDocInfo &info = it->second;
	if (dsh.lines.size() > 0)
	{
		// Apply changes only if there is something to do:
		// otherwise the lexer has cleared the document
		info.semHiglFirstLine = 0;
		info.semHiglLastLine = dsh.lines.back().line;
		applySemanticHighlighting(info);
	}
}

void LspClientLogic::applySemanticHighlighting(const LspScintillaDoc &doc)
{
	const auto &it = documents.find(doc);
	if (it != documents.end())
		applySemanticHighlighting(it->second);
}

void LspClientLogic::notifyLspFileStatus(const LspScintillaDoc &doc, const LspClangdFileStatus &sts)
{
	if (client->notifier)
		client->notifier->notifyLspFileStatus(doc, sts);
}

bool LspClientLogic::removeDocument(const LspScintillaDoc &doc)
{
	const auto &it = documents.find(doc);
	if (it == documents.end())
		// Error!
		return true;	// No references to doc
	it->second.refCount--;
	if (it->second.refCount > 0)
		return false;
	// No more references to document: remove it
	lspRemoveDocument(it->second);
	documents.erase(it);
	return true;
}

uintptr_t LspClientLogic::documentSymbolRequest(const LspScintillaDoc &doc)
{
	const auto &it = documents.find(doc);
	if (it == documents.end())
		return 0;
	return LspMessages::DocumentSymbolsRequest::send(messageHandler, &it->second);
}

uintptr_t LspClientLogic::completionRequest(const LspScintillaDoc &doc, Sci_Position pos)
{
	const auto &it = documents.find(doc);
	if (it == documents.end())
		return 0;
	return LspMessages::DocumentCompletionRequest::send(messageHandler, &it->second, LspPosition(doc, pos));
}

uintptr_t LspClientLogic::signatureHelpRequest(const LspScintillaDoc &doc, Sci_Position pos)
{
	const auto &it = documents.find(doc);
	if (it == documents.end())
		return 0;
	return LspMessages::DocumentSignatureHelpRequest::send(messageHandler, &it->second, LspPosition(doc, pos));
}

void LspClientLogic::notifyDocumentSymbols(uintptr_t id, const LspDocumentSymbols &s)
{
	if (client->notifier)
		client->notifier->notifyDocumentSymbols(id, s);
}

uintptr_t LspClientLogic::hoverRequest(const LspScintillaDoc &doc, Sci_Position pos)
{
	const auto &it = documents.find(doc);
	if (it == documents.end())
		return 0;
	return LspMessages::DocumentHoverRequest::send(messageHandler, &it->second, LspPosition(doc, pos));
}

void LspClientLogic::notifyLspHover(uintptr_t id, const LspHover &cl)
{
	if (client->notifier)
		client->notifier->notifyLspHover(id, cl);
}

void LspClientLogic::notifyLspCompletion(uintptr_t id, const LspCompletionList &cl)
{
	if (client->notifier)
		client->notifier->notifyLspCompletion(id, cl);
}

void LspClientLogic::notifyLspSignatureHelp(uintptr_t id, const LspSignatureHelp &help)
{
	if (client->notifier)
		client->notifier->notifyLspSignatureHelp(id, help);
}

LspScintillaDoc LspClientLogic::docFromUri(const std::string uri) const
{
	for (const auto &it : documents)
		if (it.second.uri == uri) return it.first;
	return {};
}

LspDocumentSemanticHighlighting *LspClientLogic::GetDocumentSemanticHighlighting(const LspScintillaDoc &doc)
{
	const auto &it = documents.find(doc);
	if (it == documents.end())
		return nullptr;
	if (it->second.dsh == nullptr)
		it->second.dsh = new LspDocumentSemanticHighlighting;
	return it->second.dsh;
}

void LspClientLogic::lspAddDocument(LspDocInfo &info)
{
	if (!initDone) return;	// It will be done later
	if (LspClientNotifications::DidOpenTextDocument::send(messageHandler, info))
		info.didOpen = true;
}

void LspClientLogic::lspRemoveDocument(LspDocInfo &info)
{
	if (!initDone) return;	// Nothing to do
	if (LspClientNotifications::DidCloseTextDocument::send(messageHandler, info))
		info.didOpen = false;
}

void LspClientLogic::applySemanticHighlighting(LspDocInfo &info)
{
	if (info.dsh == nullptr || info.applyingSemanticHighlighting)
		return;
	info.applyingSemanticHighlighting = true;
	client->applySemanticHighlighting(info.doc, info.dsh, info.semHiglFirstLine, info.semHiglLastLine);
	info.semHiglFirstLine = std::numeric_limits<decltype(info.semHiglFirstLine)>::max();
	info.semHiglLastLine = 0;
	info.applyingSemanticHighlighting = false;
}
