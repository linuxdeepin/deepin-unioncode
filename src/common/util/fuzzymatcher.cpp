// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "fuzzymatcher.h"

#include <QRegularExpression>
#include <QString>

/**
 * \brief Creates a regexp that represents a specified wildcard and camel hump pattern,
 *        underscores are not included.
 *
 * \param pattern the camel hump pattern
 * \param caseSensitivity case sensitivity used for camel hump matching
 * \return the regexp
 */
QRegularExpression FuzzyMatcher::createRegExp(
        const QString &pattern, FuzzyMatcher::CaseSensitivity caseSensitivity)
{
    if (pattern.isEmpty())
        return QRegularExpression();

    /*
     * This code builds a regular expression in order to more intelligently match
     * camel-case and underscore names.
     *
     * For any but the first letter, the following replacements are made:
     *   A => [a-z0-9_]*A
     *   a => (?:[a-zA-Z0-9]*_)?a
     *
     * That means any sequence of lower-case or underscore characters can preceed an
     * upper-case character. And any sequence of lower-case or upper case characters -
     * followed by an underscore can preceed a lower-case character.
     *
     * Examples: (case sensitive mode)
     *   gAC matches getActionController
     *   gac matches get_action_controller
     *
     * It also implements the fully and first-letter-only case sensitivity.
     */

    QString keyRegExp;
    QString plainRegExp;
    bool first = true;
    const QChar asterisk = '*';
    const QChar question = '?';
    const QLatin1String uppercaseWordFirst("(?<=\\b|[a-z0-9_])");
    const QLatin1String lowercaseWordFirst("(?<=\\b|[A-Z0-9_])");
    const QLatin1String uppercaseWordContinuation("[a-z0-9_]*");
    const QLatin1String lowercaseWordContinuation("(?:[a-zA-Z0-9]*_)?");
    const QLatin1String upperSnakeWordContinuation("[A-Z0-9]*_");
    keyRegExp += "(?:";
    for (const QChar &c : pattern) {
        if (!c.isLetter()) {
            if (c == question) {
                keyRegExp += '.';
                plainRegExp += ").(";
            } else if (c == asterisk) {
                keyRegExp += ".*";
                plainRegExp += ").*(";
            } else {
                const QString escaped = QRegularExpression::escape(c);
                keyRegExp += '(' + escaped + ')';
                plainRegExp += escaped;
            }
        } else if (caseSensitivity == CaseSensitivity::CaseInsensitive ||
            (caseSensitivity == CaseSensitivity::FirstLetterCaseSensitive && !first)) {

            const QString upper = QRegularExpression::escape(c.toUpper());
            const QString lower = QRegularExpression::escape(c.toLower());
            keyRegExp += "(?:";
            keyRegExp += first ? uppercaseWordFirst : uppercaseWordContinuation;
            keyRegExp += '(' + upper + ')';
            if (first) {
                keyRegExp += '|' + lowercaseWordFirst + '(' + lower + ')';
            } else {
                keyRegExp += '|' + lowercaseWordContinuation + '(' + lower + ')';
                keyRegExp += '|' + upperSnakeWordContinuation + '(' + upper + ')';
            }
            keyRegExp += ')';
            plainRegExp += '[' + upper + lower + ']';
        } else {
            if (!first) {
                if (c.isUpper())
                    keyRegExp += uppercaseWordContinuation;
                else
                    keyRegExp += lowercaseWordContinuation;
            }
            const QString escaped = QRegularExpression::escape(c);
            keyRegExp += escaped;
            plainRegExp += escaped;
        }

        first = false;
    }
    keyRegExp += ')';

    return QRegularExpression('(' + plainRegExp + ")|" + keyRegExp);
}

/*!
 * \brief Returns a list of matched character positions and their matched lengths for the
 * given regular expression \a match.
 *
 * The list is minimized by combining adjacent highlighting positions to a single position.
 */
FuzzyMatcher::HighlightingPositions FuzzyMatcher::highlightingPositions(
        const QRegularExpressionMatch &match)
{
    HighlightingPositions result;

    for (int i = 1, size = match.capturedTexts().size(); i < size; ++i) {
        // skip unused positions, they can appear because upper- and lowercase
        // checks for one character are done using two capture groups
        if (match.capturedStart(i) < 0)
            continue;

        // check for possible highlighting continuation to keep the list minimal
        if (!result.starts.isEmpty()
                && (result.starts.last() + result.lengths.last() == match.capturedStart(i))) {
            result.lengths.last() += match.capturedLength(i);
        } else {
            // no continuation, append as different chunk
            result.starts.append(match.capturedStart(i));
            result.lengths.append(match.capturedLength(i));
        }
    }

    return result;
}
