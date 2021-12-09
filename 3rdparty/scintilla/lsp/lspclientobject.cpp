// Scintilla source code edit control
/** @file lspclientobject.cpp
 ** Basic implementation of a client object (editor) for Language Server Protocol
 **/
// Copyright (c) 2019-2020 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "lspclientobject.h"
#include "ILspDocument.h"
#include "lspclient.h"
#include "lspstructures.h"

using namespace Scintilla;

class Scintilla::LspClientObjectPrivate
{
public:
	LspClientObjectPrivate() = default;

	//! \brief The client
	Scintilla::LspClient *lspClient {nullptr};
	//! \brief The doc
	Scintilla::LspScintillaDoc doc;
};

LspClientObject::LspClientObject()
	: p(new LspClientObjectPrivate)
{
}

LspClientObject::~LspClientObject()
{
	removeLspClient();
	delete p;
}

void LspClientObject::setupLspClient(Scintilla::LspClient *client, const Scintilla::LspScintillaDoc &doc)
{
	// Remove previous client
	removeLspClient();
	p->lspClient = client;
	p->doc = doc;
	p->lspClient->registerObject(p->doc, this);
}

void LspClientObject::setupSameClient(const LspClientObject *other)
{
	if (other == nullptr)
		return;
	setupLspClient(other->p->lspClient, other->p->doc);
}

void LspClientObject::removeLspClient()
{
	if (p->lspClient == nullptr)
		return;
	p->lspClient->unregisterObject(this);
	p->lspClient = nullptr;
}

bool LspClientObject::clientConnected() const
{
	return p->lspClient && p->lspClient->serverStarted();
}

bool LspClientObject::characterTriggersCompletion(int ch) const
{
	return clientConnected() && p->lspClient->characterTriggersCompletion(ch);
}

bool LspClientObject::characterTriggersSignatureHelpRequest(int ch) const
{
	return clientConnected() && p->lspClient->characterTriggersSignatureHelpRequest(ch);
}

void LspClientObject::documentSymbolRequest()
{
	if (p->lspClient)
		p->lspClient->documentSymbolRequest(this, p->doc);
}

void LspClientObject::hoverRequest(Sci_Position pos)
{
	if (p->lspClient)
		p->lspClient->hoverRequest(this, p->doc, pos);
}

void LspClientObject::completionRequest(Sci_Position pos)
{
	if (p->lspClient)
		p->lspClient->completionRequest(this, p->doc, pos);
}

void LspClientObject::signatureHelpRequest(Sci_Position pos)
{
	if (p->lspClient)
		p->lspClient->signatureHelpRequest(this, p->doc, pos);
}
