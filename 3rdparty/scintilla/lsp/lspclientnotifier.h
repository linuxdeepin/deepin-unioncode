// Scintilla source code edit control
/** @file lspclientnotifier.h
 ** Definition of the interface for interacting with a client object for Language Server Protocol
 **/
// Copyright (c) 2019-2020 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_LSP_CLIENTNOTIFIER_H
#define SCINTILLA_LSP_CLIENTNOTIFIER_H

#include "ILspDocument.h"

#include <map>
#include <list>

namespace Scintilla
{

class ILspClientObject;
class LspScintillaDoc;
struct LspCompletionList;
struct LspDocumentSymbols;
struct LspHover;
struct LspSignatureHelp;
struct LspDocumentDiagnostic;
struct LspClangdFileStatus;

/**
 *	\brief Notify answers from LSP server to objects (editors)
 */
class LspClientNotifier
{
public:
	~LspClientNotifier();

	//! \brief Add an object
	void registerObject(const LspScintillaDoc &doc, ILspClientObject *object);
	//! \brief remove an object
	void unregisterObject(const ILspClientObject *object);

	/**
	 *	\brief Register a request from an object
	 */
	void registerRequest(uintptr_t id, ILspClientObject *object);
	/**
	 *	\brief textDocument/publishDiagnostics notification from LSP server
	 */
	void notifyLspDiagnostic(const LspScintillaDoc &doc, const LspDocumentDiagnostic &dd);
	/**
	 *	\brief Answer to textDocument/documentSymbol from LSP server
	 */
	void notifyDocumentSymbols(uintptr_t id, const LspDocumentSymbols &s);
	/**
	 *	\brief Answer to textDocument/hover from LSP server
	 */
	void notifyLspHover(uintptr_t id, const LspHover &ho);
	/**
	 *	\brief Answer to textDocument/completion from LSP server
	 */
	void notifyLspCompletion(uintptr_t id, const LspCompletionList &cl);
	/**
	 *	\brief Answer to textDocument/signatureHelp from LSP server
	 */
	void notifyLspSignatureHelp(uintptr_t id, const LspSignatureHelp &sh);
	/**
	 *	\brief File status received from server
	 */
	void notifyLspFileStatus(const LspScintillaDoc &doc, const LspClangdFileStatus &sts);

private:
	std::map<uintptr_t, ILspClientObject *> requests;
	std::list<std::pair<uintptr_t, ILspClientObject *>> objects;
};

}

#endif	// SCINTILLA_LSP_CLIENTNOTIFIER_H
