// Scintilla source code edit control
/** @file LspScintillaEdit.h
 ** Specialization of Qt scintilla editor for LSP
 **/
// Copyright (c) 2019-2020 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_LSPSCINTILLAEDIT_H
#define SCINTILLA_LSPSCINTILLAEDIT_H

#include "../ScintillaEdit/ScintillaEdit.h"
#include "../../lsp/ILspDocument.h"
#include "../../lsp/lsp_dll.h"

class qtLspClientObject;

namespace Scintilla {
class LspClient;
class LspScintillaDoc;
struct LspCompletionList;
struct LspDocumentDiagnostic;
struct LspDocumentSymbols;
struct LspHover;
struct LspSignatureHelp;
struct LspClangdFileStatus;
}

class SCINTILLA_LSP_DLL LspScintillaEdit : public ScintillaEdit
{
	Q_OBJECT

public:
	LspScintillaEdit(QWidget *parent = Q_NULLPTR);
	~LspScintillaEdit() Q_DECL_OVERRIDE;

	/*!
	 *	\brief Set this editor to use the same document as the other editor
	 *	\details Handle LSP for both editors with the same \ref LspClient
	 *	\note The document update is done by the mainEditor,
	 *	it should *not* be destroyed.
	 */
	void useEditorDocument(const LspScintillaEdit *mainEditor);
	//! Setup LSP client
	Scintilla::LspScintillaDoc setupLspClient(Scintilla::LspClient *client);
	//! Remove LSP client
	void removeLspClient();
	//! Test LSP connected
	bool lspConnected() const;

	/**
	 *	\name LSP requests
	 *	@{
	 */
	//! Test if character triggers LSP completion request
	bool characterTriggersLspCompletion(int ch) const;
	//! Test if character triggers LSP signature help request
	bool characterTriggersLspSignatureHelpRequest(int ch) const;

	//! \brief Start document symbol request
	void lspDocumentSymbolRequest();
	//! \brief Start hover request
	void lspHoverRequest(Sci_Position pos);
	//! \brief Start completion request
	void lspCompletionRequest(Sci_Position pos);
	//! \brief Start signature help request
	void lspSignatureHelpRequest(Sci_Position pos);

	/**
	 *	@}
	 *	\name LSP notifications
	 *	@{
	 */
signals:
	void lspDiagnostic(const Scintilla::LspDocumentDiagnostic &dd);
	void lspDocumentSymbols(const Scintilla::LspDocumentSymbols &s);
	void lspHover(const Scintilla::LspHover &ho);
	void lspCompletion(const Scintilla::LspCompletionList &cl);
	void lspSignatureHelp(const Scintilla::LspSignatureHelp &help);
	void lspFileStatus(const QString &sts);

	/**	@} */

private:
	//! Lsp interface object
	qtLspClientObject *const lspObj;
	//! \brief Connection of ScintillaEditBase::notify signal to lsp client
	QMetaObject::Connection lspClientConnection;
};

#endif	// SCINTILLA_LSPSCINTILLAEDIT_H
