// Scintilla source code edit control
/** @file lspclient.h
 ** Base definition of a client for Language Server Protocol
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_LSP_CLIENT_H
#define SCINTILLA_LSP_CLIENT_H

#include "lsp_dll.h"

#include "lspclientnotifier.h"
#include "../include/Sci_Position.h"

#include <map>
#include <string>
#include <list>

struct SCNotification;

namespace Scintilla
{

class ILspClientObject;
class LspClientLogic;
class LspMessageHandler;
class LspScintillaDoc;
struct LspDocInfo;
struct LspCompletionList;
struct LspDocumentSymbols;
struct LspHover;
struct LspSignatureHelp;
struct LspDocumentDiagnostic;
struct LspDocumentSemanticHighlighting;
struct LspClangdFileStatus;

/**
 *	\brief Structure for client configuration
 */
struct LspClientConfiguration
{
	LspClientConfiguration() = default;
	enum MarkupKind
	{
		PlainText,
		Markdown
	};
	inline static MarkupKind fromString(const std::string &text)
	{
		return (text == "markdown") ? Markdown : PlainText;
	}
	static std::string toString(MarkupKind fmt)
	{
		return (fmt == Markdown ? "markdown" : "plaintext");
	}

	//! \brief Root uri
	std::string rootUri;
	//! textDocument/semanticHighlightingCapabilities
	struct
	{
		bool supported {false};				// TODO: semantic highlighting handling is broken, fix it
	} semanticHighlighting;
	//! textDocument/publishDiagnostics
	struct
	{
		bool supported {true};
		bool relatedInformation {true};
		bool categorySupport {false};		// clangd extension
		bool codeActionsInline {false};		// clangd extension
	} publishDiagnostics;
	//! textDocument/documentSymbol
	struct
	{
		bool supported {true};
		bool hierarchicalDocumentSymbolSupport {false};
	} documentSymbol;
	//! textDocument/hover
	struct
	{
		bool supported {true};
		MarkupKind contentFormat {PlainText};
	} hover;
	//! textDocument/signatureHelp
	struct
	{
		bool supported {true};
		struct
		{
			MarkupKind documentationFormat {PlainText};
			struct
			{
				bool labelOffsetSupport {true};
			} parameterInformation;
		} signatureInformation;
	} signatureHelp;

	//! workspace/didChangeWatchedFiles
	struct
	{
		bool supported {true};
	} didChangeWatchedFiles;
	//! workspace/symbol
	struct
	{
		bool supported {true};
	} symbol;
	//! workspace/executeCommand
	struct
	{
		bool supported {false};
	} executeCommand;
	//! workspace/configuration
	struct
	{
		bool supported {false};
	} configuration;
	//! workspace/workspaceFolders
	struct
	{
		bool supported {false};
	} workspaceFolders;

	//! clangd.fileStatus
	bool clangdFileStatus {false};
};

/**
 *	\brief LSP client interface
 *	\details This is the main interface to use LSP with Scintilla.
 */
class SCINTILLA_LSP_DLL LspClient
{
	friend class LspClientLogic;

public:
	enum
	{
		//! \brief First style used for semantic highlighting
		FirstLspStyle = 100,
	};

	LspClient();
	LspClient(const LspClient &) = delete;
	LspClient(LspClient &&) = delete;
	virtual ~LspClient();

	//! \brief Style to be used for disabled lines (default is SCI_DEFAULT)
	void setStyleForDisabledLines(char style) { styleDisabledLines = style; }

	/**
	 *	\name Methods for managing client objects
	 *	A client object is a class implementig the interface \ref ILspClientObject
	 *	Normally it is and editor, could be a generic object interested in
	 *	LSP informations for a document
	 *	@{
	 */

	//! \brief Add an object
	void registerObject(const LspScintillaDoc &doc, ILspClientObject *object);
	//! \brief remove an object
	void unregisterObject(const ILspClientObject *object);

	/**
	 *	@}
	 *	\name Methods for starting/stopping the server
	 *	@{
	 */

	//! \brief Start the server executable
	bool lspStartServer(const char *exec, const char *params);
	//! \brief Test server start state
	bool serverStarted() const;
	//! \brief Execute server initialization
	bool lspInitialize(const LspClientConfiguration &cfg);
	//! \brief Stop the server
	void lspStopServer();

	/**	@} */

	/**
	 *	\name Methods for managing documents
	 *	@{
	 */

	//! \brief Add document
	void addDocument(const LspScintillaDoc &doc, const std::string &uri, const std::string &languageId);
	/**
	 *	\brief Remove document
	 *	\return Returns true if the document was removed (no more references to it)
	 */
	bool removeDocument(const LspScintillaDoc &doc);

	/**
	 *	@}
	 *	\name Methods for querying opened documents
	 *	@{
	 */

	//! \brief textDocument/documentSymbol
	uintptr_t documentSymbolRequest(ILspClientObject *obj, const LspScintillaDoc &doc);

	//! \brief textDocument/hover
	uintptr_t hoverRequest(ILspClientObject *obj, const LspScintillaDoc &doc, Sci_Position pos);

	//! \brief textDocument/completion
	uintptr_t completionRequest(ILspClientObject *obj, const LspScintillaDoc &doc, Sci_Position pos);
	//! \brief Test if ch does trigger code completion
	bool characterTriggersCompletion(char ch) const;

	//! \brief textDocument/signatureHelp
	uintptr_t signatureHelpRequest(ILspClientObject *obj, const LspScintillaDoc &doc, Sci_Position pos);
	//! \brief Test if ch does trigger code completion
	bool characterTriggersSignatureHelpRequest(char ch) const;

	/**
	 *	\brief Update LSP status
	 *	\details This function must be called by the implementation
	 *	to handle the update of the document status
	 */
	void onDocumentNotification(const LspScintillaDoc &doc, const SCNotification &scn);

	/**	@} */

protected:
	//! \brief Shortcut to call the right version (needs info from \ref clientLogic)
	void applySemanticHighlighting(const LspScintillaDoc &doc);
	/**
	 *	\brief Apply semantic highlighting to a document
	 *	\details The default implementation should work in all cases
	 */
	virtual void applySemanticHighlighting(
		const LspScintillaDoc &doc,
		LspDocumentSemanticHighlighting *dsh,
		Sci_Position startLine,
		Sci_Position endLine);

private:
	//! \brief Called (by the client logic) when server started
	virtual void onServerStarted() {}

private:
	//! \brief Object to notify server answers
	LspClientNotifier *notifier;
	//! \brief Object to handle client logic
	LspClientLogic *const clientLogic;
	//! \brief Style to apply to disabled lines
	char styleDisabledLines;
};

}

#endif	// SCINTILLA_LSP_CLIENT_H
