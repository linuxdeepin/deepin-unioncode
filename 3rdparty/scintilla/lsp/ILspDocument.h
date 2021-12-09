// Scintilla source code edit control
/** @file ILspDocument.h
 ** Scintilla document interface
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_LSP_IDOCUMENT_H
#define SCINTILLA_LSP_IDOCUMENT_H

#include "lsp_dll.h"

#include "../include/Scintilla.h"

namespace Scintilla
{

class Document;

class SCINTILLA_LSP_DLL LspScintillaDoc
{
public :
	LspScintillaDoc();
	LspScintillaDoc(sptr_t doc_p);
	LspScintillaDoc(const LspScintillaDoc &other);

	operator uintptr_t() const = delete;
	operator sptr_t() const = delete;
	uintptr_t getDoc() const { return reinterpret_cast<uintptr_t>(doc); }
	bool operator < (const LspScintillaDoc &other) const { return doc < other.doc; }
	const LspScintillaDoc &operator = (const LspScintillaDoc &other) { doc = other.doc; return *this; }
	bool isNull() const { return doc == nullptr; }

	Sci_Position Length() const;
	Sci_Position LineStart(Sci_Position line) const;
	Sci_Position LineEnd(Sci_Position line) const;
	Sci_Position LineFromPosition(Sci_Position pos) const;
	Sci_Position CountUTF16(Sci_Position startPos, Sci_Position endPos) const noexcept;
	Sci_Position GetRelativePositionUTF16(Sci_Position positionStart, Sci_Position characterOffset) const noexcept;
	void GetCharRange(char *buffer, Sci_Position position, Sci_Position lengthRetrieve) const;

	void AnnotationSetText(Sci_Position line, const char *text) const;
	void AnnotationClearAll() const;

	void StartStyling(Sci_Position position) const;
	bool SetStyleFor(Sci_Position length, char style) const;

private:
	Document *doc;
};

}

#endif	// SCINTILLA_LSP_IDOCUMENT_H
