// Scintilla source code edit control
/** @file ILspDocument.cpp
 ** Scintilla document interface
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "ILspDocument.h"

#include <cstring>
#include <string>
#include <vector>
#include <forward_list>

#include "../include/Scintilla.h"
#include "../include/Platform.h"

// All this just to include Document.h
#include "../src/Position.h"
#include "../src/CaseFolder.h"
#include "../lexlib/CharacterCategory.h"
#include "../include/ILoader.h"
#include "../src/SplitVector.h"
#include "../src/Partitioning.h"
#include "../src/RunStyles.h"
#include "../src/Decoration.h"
#include "../src/CellBuffer.h"
#include "../src/PerLine.h"
#include "../include/ILexer.h"
#include "../src/CharClassify.h"
#include "../src/Document.h"

using namespace Scintilla;

LspScintillaDoc::LspScintillaDoc()
	: doc(nullptr)
{
}

LspScintillaDoc::LspScintillaDoc(sptr_t doc_p)
	: doc(reinterpret_cast<Document *>(doc_p))
{
}

LspScintillaDoc::LspScintillaDoc(const LspScintillaDoc &other)
	: doc(other.doc)
{
}

Sci_Position LspScintillaDoc::Length() const
{
	return doc ? doc->Length() : 0;
}

Sci_Position LspScintillaDoc::LineStart(Sci_Position line) const
{
	return doc ? doc->LineStart(line) : 0;
}

Sci_Position LspScintillaDoc::LineEnd(Sci_Position line) const
{
	return doc ? doc->LineEnd(line) : 0;
}

Sci_Position LspScintillaDoc::LineFromPosition(Sci_Position pos) const
{
	return doc ? doc->LineFromPosition(pos) : 0;
}

Sci_Position LspScintillaDoc::CountUTF16(Sci_Position startPos, Sci_Position endPos) const noexcept
{
	return doc ? doc->CountUTF16(startPos, endPos) : 0;
}

Sci_Position LspScintillaDoc::GetRelativePositionUTF16(Sci_Position positionStart, Sci_Position characterOffset) const noexcept
{
	return doc ? doc->GetRelativePositionUTF16(positionStart, characterOffset) : 0;
}

void LspScintillaDoc::GetCharRange(char *buffer, Sci_Position position, Sci_Position lengthRetrieve) const
{
	if (doc)
		doc->GetCharRange(buffer, position, lengthRetrieve);
	else
		std::memset(buffer, 0, lengthRetrieve);
}

void LspScintillaDoc::AnnotationSetText(Sci_Position line, const char *text) const
{
	if (doc) doc->AnnotationSetText(line, text);
}

void LspScintillaDoc::AnnotationClearAll() const
{
	if (doc) doc->AnnotationClearAll();
}

void LspScintillaDoc::StartStyling(Sci_Position position) const
{
	if (doc) doc->StartStyling(position);
}

bool LspScintillaDoc::SetStyleFor(Sci_Position length, char style) const
{
	return doc ? doc->SetStyleFor(length, style) : false;
}
