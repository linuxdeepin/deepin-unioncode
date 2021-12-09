// Scintilla source code edit control
/** @file LspScintillaEdit.cpp
 ** Specialization of Qt scintilla editor for LSP
 **/
// Copyright (c) 2019-2020 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "LspScintillaEdit.h"
#include "../ScintillaLsp/qtlspclientobject.h"
#include "../../lsp/lspclient.h"

LspScintillaEdit::LspScintillaEdit(QWidget *parent)
	: ScintillaEdit(parent),
	lspObj(new qtLspClientObject(this))
{
	lspObj->connectSignals(this);
}

LspScintillaEdit::~LspScintillaEdit()
{
	removeLspClient();
}

void LspScintillaEdit::useEditorDocument(const LspScintillaEdit *mainEditor)
{
	removeLspClient();
	const uintptr_t doc = mainEditor->docPointer();
	setDocPointer(doc);
	lspObj->setupSameClient(mainEditor->lspObj);
}

Scintilla::LspScintillaDoc LspScintillaEdit::setupLspClient(Scintilla::LspClient *client)
{
	removeLspClient();
	const Scintilla::LspScintillaDoc doc(docPointer());
	lspObj->setupLspClient(client, doc);
	lspClientConnection = connect(this, &ScintillaEditBase::notify, [=](SCNotification *scn) {
		client->onDocumentNotification(doc, *scn);
	});
	// TEST
	send(SCI_SETIDLESTYLING, SC_IDLESTYLING_NONE);
#ifdef DEBUG
	send(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_BOXED);
#endif
	return doc;
}

void LspScintillaEdit::removeLspClient()
{
	disconnect(lspClientConnection);
	lspObj->removeLspClient();
}

bool LspScintillaEdit::lspConnected() const
{
	return lspObj->clientConnected();
}

bool LspScintillaEdit::characterTriggersLspCompletion(int ch) const
{
	return lspObj->characterTriggersCompletion(ch);
}

bool LspScintillaEdit::characterTriggersLspSignatureHelpRequest(int ch) const
{
	return lspObj->characterTriggersSignatureHelpRequest(ch);
}

void LspScintillaEdit::lspDocumentSymbolRequest()
{
	lspObj->documentSymbolRequest();
}

void LspScintillaEdit::lspHoverRequest(Sci_Position pos)
{
	lspObj->hoverRequest(pos);
}

void LspScintillaEdit::lspCompletionRequest(Sci_Position pos)
{
	lspObj->completionRequest(pos);
}

void LspScintillaEdit::lspSignatureHelpRequest(Sci_Position pos)
{
	lspObj->signatureHelpRequest(pos);
}
