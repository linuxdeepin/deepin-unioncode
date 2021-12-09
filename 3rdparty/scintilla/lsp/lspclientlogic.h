// Scintilla source code edit control
/** @file lspclientlogic.h
 ** Handles the logic of a client communicating with a Language Server
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_LSP_CLIENTLOGIC_H
#define SCINTILLA_LSP_CLIENTLOGIC_H

#include "lsp_dll.h"

#include "lspclient.h"
#include "ILspDocument.h"
#include "../include/Sci_Position.h"

#include <map>

struct SCNotification;

namespace Scintilla
{

class LspMessageHandler;
struct LspCompletionList;
struct LspDocumentDiagnostic;
struct LspDocumentSemanticHighlighting;
struct LspServerCapabilities;
struct LspSignatureHelp;

//! Informations about every single managed document
struct LspDocInfo
{
	LspDocInfo(const LspScintillaDoc &d, const std::string &_uri, const std::string &_langId);
	LspDocInfo(const LspDocInfo &cp) = delete;
	LspDocInfo(LspDocInfo &&mv) = delete;
	~LspDocInfo();

	LspScintillaDoc doc;
	bool didOpen;
	int refCount;
	std::string uri;
	std::string languageId;
	int version;
	LspDocumentSemanticHighlighting *dsh;
	//! Flag to avoid recursion
	bool applyingSemanticHighlighting;
	//! \brief Scheduled first line for redoing semantic highlighting
	Sci_Position semHiglFirstLine;
	//! \brief Scheduled last line for redoing semantic highlighting
	Sci_Position semHiglLastLine;
};

/**
 *	\brief Core handling of the client work
 *	\details All implementations must call \ref onDocumentNotification()
 *	directly from the handler of notifyParent()
 */
class LspClientLogic
{
public:
	LspClientLogic(LspClient *client);
	~LspClientLogic();

	inline LspMessageHandler *GetMessageHandler() const { return messageHandler; }

	bool startServer(const char *exec, const char *params);
	bool serverStarted() const;
	//! Called when the server process is ready
	void onServerStarted();
	void stopServer();

	bool initialize(const LspClientConfiguration &cfg);
	void setInitDone();

	void addDocument(const LspScintillaDoc &doc, const std::string &uri, const std::string &languageId);
	//! Remove document. Returns true if no more references to doc
	bool removeDocument(const LspScintillaDoc &doc);
	inline bool hasDocument(const LspScintillaDoc &doc) const { return documents.find(doc) != documents.end(); }

	/*!
	 *	\brief Handle notifications from documents
	 */
	void onDocumentNotification(const LspScintillaDoc &doc, const SCNotification &scn);

	//! \brief Diagnostics notification from LSP server
	void notifyLspDiagnostic(const LspScintillaDoc &doc, const LspDocumentDiagnostic &dd);
	//! \brief Semantic highlighting notification from LSP server
	void notifyUpdateSemanticHighlighting(const LspScintillaDoc &doc, const LspDocumentSemanticHighlighting &dsh);
	//! \brief Apply semantic highlighting for the document
	void applySemanticHighlighting(const LspScintillaDoc &doc);
	/*!
	 *	\brief Notify interlan file status change
	 *	\note This notification is only sent from clangd
	 */
	void notifyLspFileStatus(const LspScintillaDoc &doc, const LspClangdFileStatus &sts);

	/**
	 *	\name Methods for requests to LSP server
	 *	These methods can be called to obtain informations from LSP server:
	 *	every request is identified by the used communication ID: it can be used
	 *	to route the answer to the right object (e.g. editor view).\n
	 *	0 means error: the request was not sent to the server
	 *	@{
	 */

	//! \brief textDocument/documentSymbol
	uintptr_t documentSymbolRequest(const LspScintillaDoc &doc);
	//! \brief Answer to textDocument/documentSymbol
	void notifyDocumentSymbols(uintptr_t id, const LspDocumentSymbols &s);

	//! \brief textDocument/hover
	uintptr_t hoverRequest(const LspScintillaDoc &doc, Sci_Position pos);
	//! \brief Answer to textDocument/hover
	void notifyLspHover(uintptr_t id, const LspHover &ho);

	//! \brief textDocument/completion
	uintptr_t completionRequest(const LspScintillaDoc &doc, Sci_Position pos);
	//! \brief Answer to textDocument/completion
	void notifyLspCompletion(uintptr_t id, const LspCompletionList &cl);

	//! \brief textDocument/signatureHelp
	uintptr_t signatureHelpRequest(const LspScintillaDoc &doc, Sci_Position pos);
	//! \brief Answer to textDocument/signatureHelp
	void notifyLspSignatureHelp(uintptr_t id, const LspSignatureHelp &help);

	/**	@} */

	LspScintillaDoc docFromUri(const std::string uri) const;
	LspServerCapabilities &GetServerCapabilities() { return *serverCapabilities; }
	const LspServerCapabilities &GetServerCapabilities() const { return *serverCapabilities; }
	LspDocumentSemanticHighlighting *GetDocumentSemanticHighlighting(const LspScintillaDoc &doc);

protected:
	//! Inform the server about the new document
	void lspAddDocument(LspDocInfo &info);
	//! Inform the server the document is no more managed
	void lspRemoveDocument(LspDocInfo &info);

private:
	//! Apply semantic highlighting to a document (helper function)
	void applySemanticHighlighting(LspDocInfo &info);

private:
	//! The client
	LspClient *const client;
	//! The message handler
	LspMessageHandler *const messageHandler;

	//! The list of the managed documents
	std::map<LspScintillaDoc, LspDocInfo> documents;

	//! Server capabilities
	LspServerCapabilities *const serverCapabilities;
	//! Status: init done
	bool initDone;
};

}

#endif	// SCINTILLA_LSP_CLIENTLOGIC_H
