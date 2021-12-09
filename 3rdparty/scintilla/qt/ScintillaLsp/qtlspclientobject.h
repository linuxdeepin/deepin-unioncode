// Scintilla source code edit control
/** @file qtlspclientobject.h
 ** Specialization of a client object for Language Server Protocol for Qt 
 **/
// Copyright (c) 2019-2020 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_LSP_QTCLIENTOBJECT_H
#define SCINTILLA_LSP_QTCLIENTOBJECT_H

#include "../../lsp/lspclientobject.h"
#include "../../include/Scintilla.h"
#include "../../lsp/lsp_dll.h"

#include <QObject>

namespace Scintilla
{
class LspClient;
class LspScintillaDoc;
}

class SCINTILLA_LSP_DLL qtLspClientObject : public QObject, public Scintilla::LspClientObject
{
	Q_OBJECT
	
public:
	qtLspClientObject(QObject *parent);
	~qtLspClientObject() Q_DECL_OVERRIDE;

	template <class editor>
	void connectSignals(editor *e)
	{
		connect(this, &qtLspClientObject::lspDiagnostic, e, &editor::lspDiagnostic);
		connect(this, &qtLspClientObject::lspDocumentSymbols, e, &editor::lspDocumentSymbols);
		connect(this, &qtLspClientObject::lspHover, e, &editor::lspHover);
		connect(this, &qtLspClientObject::lspCompletion, e, &editor::lspCompletion);
		connect(this, &qtLspClientObject::lspSignatureHelp, e, &editor::lspSignatureHelp);
		connect(this, &qtLspClientObject::lspFileStatus, e, &editor::lspFileStatus);
	}

signals:
	void lspDiagnostic(const Scintilla::LspDocumentDiagnostic &dd);
	void lspDocumentSymbols(const Scintilla::LspDocumentSymbols &s);
	void lspHover(const Scintilla::LspHover &ho);
	void lspCompletion(const Scintilla::LspCompletionList &cl);
	void lspSignatureHelp(const Scintilla::LspSignatureHelp &sh);
	void lspFileStatus(const QString &sts);

	/**
	 *	\name Implementation of \ref Scintilla::LspClientObject interface
	 *	@{
	 */
private:
	void notifyLspDiagnostic(const Scintilla::LspDocumentDiagnostic &dd) Q_DECL_OVERRIDE;
	void notifyDocumentSymbols(const Scintilla::LspDocumentSymbols &s) Q_DECL_OVERRIDE;
	void notifyLspHover(const Scintilla::LspHover &ho) Q_DECL_OVERRIDE;
	void notifyLspCompletion(const Scintilla::LspCompletionList &cl) Q_DECL_OVERRIDE;
	void notifyLspSignatureHelp(const Scintilla::LspSignatureHelp &sh) Q_DECL_OVERRIDE;
	void notifyLspFileStatus(const Scintilla::LspClangdFileStatus &sts) Q_DECL_OVERRIDE;
	/**	@} */
};

#endif	// SCINTILLA_LSP_QTCLIENTOBJECT_H

