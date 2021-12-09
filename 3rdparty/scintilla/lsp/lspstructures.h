// Scintilla source code edit control
/** @file lspstructures.h
 ** Class for converting from json objects to structures
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_LSP_STRUCTURES_H
#define SCINTILLA_LSP_STRUCTURES_H

#include "lspmessages.h"

#include "../include/Scintilla.h"
#include "lsp_dll.h"

#include <vector>

namespace Scintilla {

class LspScintillaDoc;

template <class T> T lspDefaultValue() { return T(); }
template<> inline bool lspDefaultValue() { return false; }
template<> inline int lspDefaultValue() { return 0; }

template <class T>
bool lspGetValue(T &var, const lspMessage &msg, const char *field)
{
	const auto &it = msg.find(field);
	if (it != msg.end())
	{
		var = it->get<T>();
		return true;
	}
	var = lspDefaultValue<T>();
	return false;
}

struct LspTextDocumentIdentifier
{
	LspTextDocumentIdentifier() : isVersioned(false), version(0) {}
	bool fromMsg(const lspMessage &msg)
	{
		const auto &it_version = msg.find("version");
		isVersioned = (it_version != msg.end());
		version = isVersioned ? it_version->get<int>() : 0;
		return lspGetValue(uri, msg, "uri");
	}

	bool isVersioned;
	int version;
	std::string uri;
};

struct LspPosition
{
	LspPosition() : line(-1), character(0) {}
	LspPosition(const LspScintillaDoc &doc, Sci_Position pos);
	bool fromMsg(const lspMessage &msg)
	{
		const auto &it_line = msg.find("line");
		const auto &it_character = msg.find("character");
		if ((it_line == msg.end()) || (it_character == msg.end()))
			return false;
		line = it_line->get<int>();
		character = it_character->get<int>();
		return true;
	}
	lspMessage toMsg() const { return lspMessage({{ "line" , line}, { "character", character }}); }

	int line;
	int character;
};

struct LspRange
{
	LspRange() = default;
	bool fromMsg(const lspMessage &msg)
	{
		const auto &it_start = msg.find("start");
		const auto &it_end = msg.find("end");
		if ((it_start == msg.end()) || (it_end == msg.end()))
			return false;
		return (start.fromMsg(it_start.value()) && end.fromMsg(it_end.value()));
	}

	LspPosition start;
	LspPosition end;
};

//! Conversions from Scintilla to lsp and vice versa
class SCINTILLA_LSP_DLL lspConv
{
public:
	static Sci_Position sciPos(const LspScintillaDoc &doc, int line, int character);
	static inline Sci_Position sciPos(const LspScintillaDoc &doc, const LspPosition &pos) {
		return sciPos(doc, pos.line, pos.character);
	}

	static lspMessage Position(const LspScintillaDoc &doc, Sci_Position pos);
	static lspMessage Range(const LspScintillaDoc &doc, Sci_Position pos, sptr_t len);
	static bool fromPos(const lspMessage &msg, const LspScintillaDoc &doc, Sci_Position &pos);
	static bool fromRange(const lspMessage &msg, const LspScintillaDoc &doc, Sci_Position &p_start, Sci_Position &p_end);
	//! \brief Convert a range to p_start and p_stop positions in the document
	static bool convertRange(const LspScintillaDoc &doc, const LspRange &range, Sci_Position &p_start, Sci_Position &p_end);
};

struct LspLocation
{
	LspLocation() = default;

	bool fromMsg(const lspMessage &msg)
	{
		const auto &it_uri = msg.find("uri");
		const auto &it_range = msg.find("range");
		if ((it_uri == msg.end()) || (it_range == msg.end()))
			return false;
		uri = it_uri->get<std::string>();
		range.fromMsg(it_range.value());
		return true;
	}

	std::string uri;
	LspRange range;
};

struct LspServerCapabilities
{
	LspServerCapabilities()
		: codeActionProvider(false),
		declarationProvider(false),
		definitionProvider(false),
		documentFormattingProvider(false),
		documentHighlightProvider(false),
		documentRangeFormattingProvider(false),
		hoverProvider(false),
		referencesProvider(false),
		renameProvider(false),
		selectionRangeProvider(false),
		textDocumentSync(false),
		typeHierarchyProvider(false),
		workspaceSymbolProvider(false)
	{
	}
	void fromMsg(const lspMessage &msg);

public:
	bool codeActionProvider;
	struct _completionProvider
	{
		_completionProvider() = default;
		bool resolveProvider {false};
		std::string triggerCharacters;
	} completionProvider;
	bool declarationProvider;
	bool definitionProvider;
	bool documentFormattingProvider;
	bool documentHighlightProvider;
	struct _documentOnTypeFormattingProvider
	{
		_documentOnTypeFormattingProvider() = default;
		char firstTriggerCharacter {' '};
		std::string moreTriggerCharacter;
	} documentOnTypeFormattingProvider;
	bool documentRangeFormattingProvider;
	struct _executeCommandProvider
	{
		_executeCommandProvider() = default;
		std::vector<std::string> commands;
	} executeCommandProvider;
	bool hoverProvider;
	bool referencesProvider;
	bool renameProvider;
	bool selectionRangeProvider;
	bool semanticHighlighting;
	std::vector<std::string> semanticHighlightingScopes;
	bool signatureHelpProvider;
	struct LspSignatureHelpOptions
	{
		LspSignatureHelpOptions() = default;
		void fromMsg(const lspMessage &options);
		std::string triggerCharacters;
	} signatureHelpOptions;
	int textDocumentSync;
	bool typeHierarchyProvider;
	bool workspaceSymbolProvider;
};

struct LspSymbolDef
{
	LspSymbolDef(const lspMessage &msg) : kind(-1) { fromMsg(msg); }
	bool fromMsg(const lspMessage &msg);

	int kind;
	std::string name;
	std::string containerName;
	LspLocation location;
};

struct LspDocumentSymbols: public std::vector<LspSymbolDef>
{
	LspDocumentSymbols(const lspMessage &msg) { fromMsg(msg); }
	bool fromMsg(const lspMessage &msg);
};

struct LspDiagnosticRelatedInformation
{
	LspDiagnosticRelatedInformation() = default;
	LspDiagnosticRelatedInformation(const lspMessage &msg) { fromMsg(msg); }
	bool fromMsg(const lspMessage &msg);

	LspLocation location;
	std::string message;
};

struct LspDiagnosticElement
{
	enum DiagnosticSeverity
	{
		/**
		 * Reports an error.
		 */
		Error = 1,
		/**
		 * Reports a warning.
		 */
		Warning = 2,
		/**
		 * Reports an information.
		 */
		Information = 3,
		/**
		 * Reports a hint.
		 */
		Hint = 4
	};

	bool fromMsg(const lspMessage &msg);

	LspRange range;
	int severity;
	std::string code;
	std::string source;
	std::string message;
	std::vector<LspDiagnosticRelatedInformation> relatedInformation;
};

struct LspDocumentDiagnostic : public std::vector<LspDiagnosticElement>
{
	bool fromMsg(const lspMessage &msg);
};

/**
 *	\brief Represents a reference to a command.
 *	\details Provides a title which will be used to represent a command in the UI.
 *	Commands are identitifed using a string identifier and the protocol currently
 *	doesn't specify a set of well known commands.
 *	So executing a command requires some tool extension code.
 */
struct LspCommand
{
	/**
	 *	Title of the command, like `save`.
	 */
	std::string title;
	/**
	 *	The identifier of the actual command handler.
	 */
	std::string command;
	/**
	 *	Arguments that the command handler should be
	 *	invoked with.
	 */
	//arguments ? : any[];
};

struct LspTextEdit
{
	void fromMsg(const lspMessage &msg);

	/**
	 * The range of the text document to be manipulated. To insert
	 * text into a document create a range where start === end.
	 */
	LspRange range;
	/**
	 *	The string to be inserted. For delete operations use an
	 *	empty string.
	 */
	std::string newText;
};

struct LspCompletionItem
{
	enum LspCompletionItemKind
	{
		Text = 1,
		Method = 2,
		Function = 3,
		Constructor = 4,
		Field = 5,
		Variable = 6,
		Class = 7,
		Interface = 8,
		Module = 9,
		Property = 10,
		Unit = 11,
		Value = 12,
		Enum = 13,
		Keyword = 14,
		Snippet = 15,
		Color = 16,
		File = 17,
		Reference = 18
	};

	void fromMsg(const lspMessage &msg);

	/**
	 *	The label of this completion item. By default
	 *	also the text that is inserted when selecting
	 *	this completion.
	 */
	std::string label;
	/**
	 *	The kind of this completion item. Based of the kind
	 *	an icon is chosen by the editor.
	 */
	int kind;
	/**
	 *	A human-readable string with additional information
	 *	about this item, like type or symbol information.
	 */
	std::string detail;
	/**
	 *	A human-readable string that represents a doc-comment.
	 */
	std::string documentation;
	/**
	 *	A string that should be used when comparing this item
	 *	with other items. When `falsy` the label is used.
	 */
	std::string sortText;
	/**
	 *	A string that should be used when filtering a set of
	 *	completion items. When `falsy` the label is used.
	 */
	std::string filterText;
	/**
	 *	A string that should be inserted a document when selecting
	 *	this completion. When `falsy` the label is used.
	 */
	std::string insertText;
	/**
	 *	An edit which is applied to a document when selecting
	 *	this completion. When an edit is provided the value of
	 *	insertText is ignored.
	 */
	LspTextEdit textEdit;
	/**
	 *	An optional array of additional text edits that are applied when
	 *	selecting this completion. Edits must not overlap with the main edit
	 *	nor with themselves.
	 */
	std::vector<LspTextEdit> additionalTextEdits;
	/**
	 *	An optional command that is executed *after* inserting this completion. *Note* that
	 *	additional modifications to the current document should be described with the
	 *	additionalTextEdits-property.
	 */
	LspCommand command;
	/**
	 * An data entry field that is preserved on a completion item between
	 * a completion and a completion resolve request.
	 */
	//data ? : any
};

struct LspCompletionList
{
	LspCompletionList() = default;
	LspCompletionList(const lspMessage &msg) { fromMsg(msg); }
	void fromMsg(const lspMessage &msg);
	LspCompletionList &operator = (const LspCompletionList &other) = default;

	bool isIncomplete {false};
	std::vector<LspCompletionItem> items;
};

struct LspParameterInformation
{
	LspParameterInformation() = default;
	LspParameterInformation(const lspMessage &msg) { fromMsg(msg); }
	bool fromMsg(const lspMessage &msg);

	/**
	 *	The label of this parameter information.
	 *
	 *	Either a string or an inclusive start and exclusive end offsets within its containing
	 *	signature label. (see SignatureInformation.label). The offsets are based on a UTF-16
	 *	string representation as `Position` and `Range` does.
	 *
	 *	*Note*: a label of type string should be a substring of its containing signature label.
	 *	Its intended use case is to highlight the parameter label part in the `SignatureInformation.label`.
	 */
	std::string label;
	int label_start, label_end;

	/**
	 *	The human-readable doc-comment of this parameter. Will be shown
	 *	in the UI but can be omitted.
	 */
	std::string documentation;	// TODO: maybe MarkupContent
};

struct LspSignatureInformation
{
	LspSignatureInformation() = default;
	LspSignatureInformation(const lspMessage &msg) { fromMsg(msg); }
	bool fromMsg(const lspMessage &msg);

	/**
	 *	The label of this signature. Will be shown in
	 *	the UI.
	 */
	std::string label;

	/**
	 *	The human-readable doc-comment of this signature. Will be shown
	 *	in the UI but can be omitted.
	 */
	std::string documentation;	// TODO: maybe MarkupContent

	/**
	 * The parameters of this signature.
	 */
	std::vector<LspParameterInformation> parameters;
};

struct LspSignatureHelp
{
	bool fromMsg(const lspMessage &msg);

	/**
	 *	One or more signatures.
	 */
	std::vector<LspSignatureInformation> signatures;

	/**
	 *	The active signature. If omitted or the value lies outside the
	 *	range of `signatures` the value defaults to zero or is ignored if
	 *	`signatures.length == 0`. Whenever possible implementors should
	 *	make an active decision about the active signature and shouldn't
	 *	rely on a default value.
	 *	In future version of the protocol this property might become
	 *	mandatory to better express this.
	 */
	int activeSignature;

	/**
	 *	The active parameter of the active signature. If omitted or the value
	 *	lies outside the range of `signatures[activeSignature].parameters`
	 *	defaults to 0 if the active signature has parameters. If
	 *	the active signature has no parameters it is ignored.
	 *	In future version of the protocol this property might become
	 *	mandatory to better express the active parameter if the
	 *	active signature does have any.
	 */
	int activeParameter;
};

struct LspSemanticToken
{
	int32_t character;
	int16_t length;
	int16_t scope;
};

struct SemanticHighlightingLineInfo
{
	SemanticHighlightingLineInfo() = default;
	SemanticHighlightingLineInfo(const lspMessage &msg) { fromMsg(msg); }

	inline bool validLine() const { return isInactive || (tokens.size() > 0); }
	bool fromMsg(const lspMessage &msg);
	std::vector<LspSemanticToken> decodeTokens() const;

	/**
	 *	The zero-based line position in the text document.
	 */
	Sci_Position line;
	/**
	 *	Optional flag: inactive line
	 */
	bool isInactive;

	/**
	 *	A base64 encoded string representing every single highlighted characters with its start position, length and the "lookup table" index of
	 *	of the semantic highlighting [TextMate scopes](https://manual.macromates.com/en/language_grammars).
	 *	If the `tokens` is empty or not defined, then no highlighted positions are available for the line.
	 */
	std::string tokens;
};

/**
 *	\brief Structure to read semantic highlighting definition from message
 */
struct LspSemanticHighlighting
{
	LspSemanticHighlighting() = default;
	LspSemanticHighlighting(const lspMessage &msg) { fromMsg(msg); }
	bool fromMsg(const lspMessage &msg);

	LspTextDocumentIdentifier identifier;
	std::list<SemanticHighlightingLineInfo> lines;
};

/**
 *	\brief Semantic highlighting of the whole document
 *	\details This struct keeps the highlighting of the whole document.
 *	It will be updated with the messages received from the server.
 */
struct LspDocumentSemanticHighlighting
{
	LspDocumentSemanticHighlighting() = default;
	void clear();
	void update(LspSemanticHighlighting &&data);
#if 0
	//! \brief Returns the first element referring to a line >= searchLine
	std::vector<SemanticHighlightingLineInfo>::iterator findLine(Sci_Position searchLine)
	{
		return std::lower_bound(lines.begin(), lines.end(), searchLine, [](const SemanticHighlightingLineInfo &val, Sci_Position l) {
			return (val.line < l);
		});
	}
#endif

	std::list<SemanticHighlightingLineInfo> lines;
};

/**
 * A `MarkupContent` literal represents a string value which content is interpreted base on its
 * kind flag. Currently the protocol supports `plaintext` and `markdown` as markup kinds.
 *
 * If the kind is `markdown` then the value can contain fenced code blocks like in GitHub issues.
 * See https://help.github.com/articles/creating-and-highlighting-code-blocks/#syntax-highlighting
 *
 * Here is an example how such a string can be constructed using JavaScript / TypeScript:
 * ```typescript
 * let markdown: MarkdownContent = {
 *  kind: MarkupKind.Markdown,
 *	value: [
 *		'# Header',
 *		'Some text',
 *		'```typescript',
 *		'someCode();',
 *		'```'
 *	].join('\n')
 * };
 * ```
 *
 * *Please Note* that clients might sanitize the return markdown. A client could decide to
 * remove HTML from the markdown to avoid script execution.
 */
struct LspMarkupContent
{
	LspMarkupContent() = default;
	LspMarkupContent(const lspMessage &msg) { fromMsg(msg); }
	bool fromMsg(const lspMessage &msg);

	std::string kind;
	std::string value;
};

/**
 * The result of a hover request.
 */
struct LspHover
{
	LspHover() = default;
	LspHover(const lspMessage &msg) { fromMsg(msg); }
	bool fromMsg(const lspMessage &msg);

	/**
	 *	The hover's content
	 */
	LspMarkupContent contents;

	/**
	 *	An optional range is a range inside a text document
	 *	that is used to visualize a hover, e.g. by changing the background color.
	 */
	LspRange range;
};

struct LspClangdFileStatus
{
	LspClangdFileStatus() = default;
	LspClangdFileStatus(const lspMessage &msg) { fromMsg(msg); }
	bool fromMsg(const lspMessage &msg);

	std::string uri;	// the document whose status is being updated
	std::string state;	// human - readable information about current activity.
};

}	// namespace Scintilla

#endif	// SCINTILLA_LSP_STRUCTURES_H
