// Scintilla source code edit control
/** @file qtlspclientobject.h
 ** Specialization of a client object for Language Server Protocol for Qt
 **/
// Copyright (c) 2019-2020 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "qtlspclientobject.h"

#include "../../lsp/lspclient.h"
#include "../../lsp/lspstructures.h"

qtLspClientObject::qtLspClientObject(QObject *parent)
	: QObject(parent)
{
}

qtLspClientObject::~qtLspClientObject()
{
}

void qtLspClientObject::notifyLspDiagnostic(const Scintilla::LspDocumentDiagnostic &dd)
{
	emit lspDiagnostic(dd);
}

void qtLspClientObject::notifyDocumentSymbols(const Scintilla::LspDocumentSymbols &s)
{
	emit lspDocumentSymbols(s);
}

void qtLspClientObject::notifyLspHover(const Scintilla::LspHover &ho)
{
	emit lspHover(ho);
}

void qtLspClientObject::notifyLspCompletion(const Scintilla::LspCompletionList &cl)
{
	emit lspCompletion(cl);
}

void qtLspClientObject::notifyLspSignatureHelp(const Scintilla::LspSignatureHelp &sh)
{
	emit lspSignatureHelp(sh);
}

void qtLspClientObject::notifyLspFileStatus(const Scintilla::LspClangdFileStatus &sts)
{
	emit lspFileStatus(QString::fromStdString(sts.state));
}
