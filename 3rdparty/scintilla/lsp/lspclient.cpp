// Scintilla source code edit control
/** @file lspclient.cpp
 ** Base definition of a client for Language Server Protocol
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "lspclient.h"
#include "lspclientlogic.h"
#include "lspstructures.h"
#include "ILspDocument.h"
#include "lspmessagehandler.h"

#include "../include/Platform.h"

using namespace Scintilla;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LspClient::LspClient()
	: notifier(new LspClientNotifier),
	clientLogic(new LspClientLogic(this)),
	styleDisabledLines(STYLE_DEFAULT)
{
	// Consistency check
	static_assert(FirstLspStyle > STYLE_LASTPREDEFINED);
	assert(notifier);
}

LspClient::~LspClient()
{
	delete notifier;
	notifier = nullptr;
	delete clientLogic;
}

void LspClient::registerObject(const LspScintillaDoc &doc, ILspClientObject *object)
{
	if (notifier) notifier->registerObject(doc, object);
}

void LspClient::unregisterObject(const ILspClientObject *object)
{
	if (notifier) notifier->unregisterObject(object);
}

bool LspClient::lspStartServer(const char *exec, const char *params)
{
	return clientLogic->startServer(exec, params);
}

bool LspClient::serverStarted() const
{
	return clientLogic->serverStarted();
}

bool LspClient::lspInitialize(const LspClientConfiguration &cfg)
{
	return clientLogic->initialize(cfg);
}

void LspClient::lspStopServer()
{
	clientLogic->stopServer();
}

void LspClient::addDocument(const LspScintillaDoc &doc, const std::string &uri, const std::string &languageId)
{
	clientLogic->addDocument(doc, uri, languageId);
}

bool LspClient::removeDocument(const LspScintillaDoc &doc)
{
	return clientLogic->removeDocument(doc);
}

uintptr_t LspClient::documentSymbolRequest(ILspClientObject *obj, const LspScintillaDoc &doc)
{
	if (notifier == nullptr)
		return 0;
	const auto id = clientLogic->documentSymbolRequest(doc);
	if (id != 0) notifier->registerRequest(id, obj);
	return id;
}

uintptr_t LspClient::hoverRequest(ILspClientObject *obj, const LspScintillaDoc &doc, Sci_Position pos)
{
	if (notifier == nullptr)
		return 0;
	const auto id = clientLogic->hoverRequest(doc, pos);
	if (id != 0) notifier->registerRequest(id, obj);
	return id;
}

uintptr_t LspClient::completionRequest(ILspClientObject *obj, const LspScintillaDoc &doc, Sci_Position pos)
{
	if (notifier == nullptr)
		return 0;
	const auto id = clientLogic->completionRequest(doc, pos);
	if (id != 0) notifier->registerRequest(id, obj);
	return id;
}

bool LspClient::characterTriggersCompletion(char ch) const
{
	const auto &tc = clientLogic->GetServerCapabilities().completionProvider.triggerCharacters;
	return (tc.find(ch, 0) != std::string::npos);
}

bool LspClient::characterTriggersSignatureHelpRequest(char ch) const
{
	const auto &tc = clientLogic->GetServerCapabilities().signatureHelpOptions.triggerCharacters;
	return (tc.find(ch, 0) != std::string::npos);
}

uintptr_t LspClient::signatureHelpRequest(ILspClientObject *obj, const LspScintillaDoc &doc, Sci_Position pos)
{
	if (notifier == nullptr)
		return 0;
	const auto id = clientLogic->signatureHelpRequest(doc, pos);
	if (id != 0) notifier->registerRequest(id, obj);
	return id;
}

void LspClient::onDocumentNotification(const LspScintillaDoc &doc, const SCNotification &scn)
{
	clientLogic->onDocumentNotification(doc, scn);
}

void LspClient::applySemanticHighlighting(const LspScintillaDoc &doc)
{
	clientLogic->applySemanticHighlighting(doc);
}

void LspClient::applySemanticHighlighting(
	const LspScintillaDoc &doc,
	LspDocumentSemanticHighlighting *dsh,
	Sci_Position startLine,
	Sci_Position endLine)
{
doc.AnnotationClearAll();
	// const auto &capabilities = clientLogic->GetServerCapabilities();
	for (auto s = dsh->lines.begin(); s != dsh->lines.end(); s++)
	{
		const Sci_Position l = s->line;
#ifdef DEBUG
		std::string lineAnnotation;
#else
		if (l < startLine)
			continue;
		if (l > endLine)
			break;
#endif
		const auto lineStartPos = doc.LineStart(l);
		const auto lineEndPos = doc.LineEnd(l);
		if (lineEndPos <= lineStartPos)
			continue;
		if (s->isInactive)
		{
			doc.StartStyling(lineStartPos);
			doc.SetStyleFor(lineEndPos - lineStartPos, styleDisabledLines);
#ifdef DEBUG
			// Platform::DebugPrintf("Line %d: inactive", l + 1);
			lineAnnotation = "Inactive";
#endif
		}
		const auto v = s->decodeTokens();
		if (v.size() > 0)
		{
			for (const auto &def : v)
			{
				const auto startPos = lineStartPos + def.character;
				if (startPos + def.length > lineEndPos)
					continue;
				const char style = char(FirstLspStyle + def.scope);
				doc.StartStyling(startPos);
				doc.SetStyleFor(def.length, style);
#ifdef DEBUG
				char buf[128];
				memset(buf, 0, sizeof(buf));
				snprintf(buf, sizeof(buf) - 1, "char %d length %d scope %d", def.character, def.length, def.scope);
				if (lineAnnotation.size() > 0) lineAnnotation.append(", ");
				lineAnnotation.append(buf);
				//Platform::DebugPrintf("Line %d: char %d length %d scope %d '%s'",
				//	l + 1,
				//	def.character,
				//	def.length,
				//	def.scope,
				//	capabilities.semanticHighlightingScopes.at(def.scope).c_str());
#endif
			}
		}
#ifdef DEBUG
		if (lineAnnotation.size() > 0) doc.AnnotationSetText(l, lineAnnotation.c_str());
#endif
	}
}
