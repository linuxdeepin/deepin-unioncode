// Scintilla source code edit control
/** @file lspclientobject.h
 ** Basic implementation of a client object (editor) for Language Server Protocol
 **/
// Copyright (c) 2019-2020 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_LSP_CLIENTOBJECT_H
#define SCINTILLA_LSP_CLIENTOBJECT_H

#include "ILspClientObject.h"
#include "../include/Scintilla.h"
#include "lsp_dll.h"

namespace Scintilla
{
class LspClient;
class LspScintillaDoc;
class LspClientObjectPrivate;

/**
 *	\brief Base implementation of an LSP client object
 *	\details The functions to call methods of \ref LspClient are
 *	added to the bare \ref ILspClientObject interface
 */
class SCINTILLA_LSP_DLL LspClientObject : public ILspClientObject
{
public:
	LspClientObject();
	~LspClientObject() override;

	//! \brief Connects the document to the client through this interface
	void setupLspClient(LspClient *client, const LspScintillaDoc &doc);
	//! \brief Setup this interface to use same document as other interface
	void setupSameClient(const LspClientObject *other);
	//! \brief Implementation of basic \ref Scintilla::ILspClientObject interface
	void removeLspClient() override;

	//! \brief Test client connected to LSP server
	bool clientConnected() const;
	//! \brief Returns true if client connected and character does trigger completion
	bool characterTriggersCompletion(int ch) const;
	//! \brief Returns true if client connected and character does trigger signature help request
	bool characterTriggersSignatureHelpRequest(int ch) const;

public:
	/**
	 *	\name Methods for querying LSP server through the client
	 *	@{
	 */

	//! \brief Start document symbol request
	void documentSymbolRequest();
	//! \brief Start hover request
	void hoverRequest(Sci_Position pos);
	//! \brief Start completion request
	void completionRequest(Sci_Position pos);
	//! \brief Start signature help request
	void signatureHelpRequest(Sci_Position pos);

	/**	@} */

private:
	//! \brief Hiding the implementation
	LspClientObjectPrivate *const p;
};

}

#endif	// SCINTILLA_LSP_CLIENTOBJECT_H
