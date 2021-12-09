// Scintilla source code edit control
/** @file ILspClientObject.h
 ** Definition of the interface for a client object (editor) for Language Server Protocol
 **/
// Copyright (c) 2019-2020 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_ILSPCLIENTOBJECT_H
#define SCINTILLA_ILSPCLIENTOBJECT_H

#include "lsp_dll.h"

namespace Scintilla
{
struct LspCompletionList;
struct LspDocumentSymbols;
struct LspHover;
struct LspSignatureHelp;
struct LspDocumentDiagnostic;
struct LspClangdFileStatus;

/**
 *	\brief Pure virtual interface class to react to LSP events
 *	\details This adds to an editor (or to a generic entity interested in LSP)
 *	the functions that will be called by \ref LspClient to notify the answers
 *	received from the server
 */
class SCINTILLA_LSP_DLL ILspClientObject
{
public:
	virtual ~ILspClientObject() {}
	virtual void removeLspClient() = 0;
	virtual void notifyLspDiagnostic(const LspDocumentDiagnostic &dd) = 0;
	virtual void notifyDocumentSymbols(const LspDocumentSymbols &s) = 0;
	virtual void notifyLspHover(const LspHover &ho) = 0;
	virtual void notifyLspCompletion(const LspCompletionList &cl) = 0;
	virtual void notifyLspSignatureHelp(const LspSignatureHelp &sh) = 0;
	virtual void notifyLspFileStatus(const LspClangdFileStatus &sts) = 0;
};

}

#endif	// SCINTILLA_ILSPCLIENTOBJECT_H
