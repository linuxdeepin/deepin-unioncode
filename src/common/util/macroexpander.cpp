// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "macroexpander.h"

#include <QMap>
#include <QStack>
#include <QRegularExpression>

const char FilePathPostfix[] = ":FilePath";
const char PathPostfix[] = ":Path";
const char FileNamePostfix[] = ":FileName";
const char FileBaseNamePostfix[] = ":FileBaseName";

inline static bool isSpecialCharUnix(ushort c)
{
    // Chars that should be quoted (TM). This includes:
    static const uchar iqm[] = {
        0xff, 0xff, 0xff, 0xff, 0xdf, 0x07, 0x00, 0xd8,
        0x00, 0x00, 0x00, 0x38, 0x01, 0x00, 0x00, 0x78
    };   // 0-32 \'"$`<>|;&(){}*?#!~[]

    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
}

inline static bool hasSpecialCharsUnix(const QString &arg)
{
    for (int x = arg.length() - 1; x >= 0; --x)
        if (isSpecialCharUnix(arg.unicode()[x].unicode()))
            return true;
    return false;
}

class MacroExpanderPrivate
{
public:
    enum MxQuoting {
        MxBasic,
        MxSingleQuote,
        MxDoubleQuote,
        MxParen,
        MxSubst,
        MxGroup,
        MxMath
    };

    struct MxState
    {
        MxQuoting current;
        bool dquote;
    };

    struct MxSave
    {
        QString str;
        int pos, varPos;
    };

    struct MacroInfo
    {
        MacroExpander::StringFunction cb;
        QString description;
    };

    MacroExpanderPrivate(MacroExpander *qq);

    int findMacro(const QString &str, int *pos, QString *ret);
    bool expandNestedMacros(const QString &str, int *pos, QString *ret);
    bool expandMacros(QString *cmd);

public:
    MacroExpander *q;

    QHash<QString, MacroInfo> macroMap;
    QString displayName;
};

MacroExpanderPrivate::MacroExpanderPrivate(MacroExpander *qq)
    : q(qq)
{
}

int MacroExpanderPrivate::findMacro(const QString &str, int *pos, QString *ret)
{
    forever {
        int startPos = str.indexOf("%{", *pos);
        if (startPos < 0)
            return 0;

        int varPos = startPos + 2;
        if (expandNestedMacros(str, &varPos, ret)) {
            *pos = startPos;
            return varPos - startPos;
        }

        *pos = startPos + 2;
    }
}

bool MacroExpanderPrivate::expandNestedMacros(const QString &str, int *pos, QString *ret)
{
    QString varName;
    QString pattern, replace;
    QString defaultValue;
    QString *currArg = &varName;
    QChar prev;
    QChar c;
    QChar replacementChar;
    bool replaceAll = false;

    int i = *pos;
    int strLen = str.length();
    varName.reserve(strLen - i);
    for (; i < strLen; prev = c) {
        c = str.at(i++);
        if (c == '\\' && i < strLen) {
            c = str.at(i++);
            if (currArg == &replace && c.isDigit())
                *currArg += '\\';
            *currArg += c;
        } else if (c == '}') {
            if (varName.isEmpty()) {   // replace "%{}" with "%"
                *ret = QString('%');
                *pos = i;
                return true;
            }

            bool found = false;
            *ret = q->value(varName, &found);
            if (found) {
                *pos = i;
                if (!pattern.isEmpty() && currArg == &replace) {
                    const QRegularExpression regexp(pattern);
                    if (regexp.isValid()) {
                        if (replaceAll) {
                            ret->replace(regexp, replace);
                        } else {
                            const QRegularExpressionMatch match = regexp.match(*ret);
                            if (match.hasMatch()) {
                                *ret = ret->left(match.capturedStart(0))
                                        + match.captured(0).replace(regexp, replace)
                                        + ret->mid(match.capturedEnd(0));
                            }
                        }
                    }
                }
                return true;
            }
            if (!defaultValue.isEmpty()) {
                *pos = i;
                *ret = defaultValue;
                return true;
            }
            return false;
        } else if (c == '{' && prev == '%') {
            if (!expandNestedMacros(str, &i, ret))
                return false;
            varName.chop(1);
            varName += *ret;
        } else if (currArg == &varName && c == '-' && prev == ':') {
            varName.chop(1);
            currArg = &defaultValue;
        } else if (currArg == &varName && (c == '/' || c == '#')) {
            replacementChar = c;
            currArg = &pattern;
            if (i < strLen && str.at(i) == replacementChar) {
                ++i;
                replaceAll = true;
            }
        } else if (currArg == &pattern && c == replacementChar) {
            currArg = &replace;
        } else {
            *currArg += c;
        }
    }
    return false;
}

bool MacroExpanderPrivate::expandMacros(QString *cmd)
{
    QString str = *cmd;
    if (str.isEmpty())
        return true;

    QString rsts;
    int varLen;
    int varPos = 0;
    if (!(varLen = findMacro(str, &varPos, &rsts)))
        return true;

    int pos = 0;
    MxState state = { MxBasic, false };
    QStack<MxState> sstack;
    QStack<MxSave> ostack;

    while (pos < str.length()) {
        if (pos == varPos) {
            // Our expansion rules trigger in any context
            if (state.dquote) {
                // We are within a double-quoted string. Escape relevant meta characters.
                rsts.replace(QRegularExpression(QLatin1String("([$`\"\\\\])")), QLatin1String("\\\\1"));
            } else if (state.current == MxSingleQuote) {
                // We are within a single-quoted string. "Suspend" single-quoting and put a
                // single escaped quote for each single quote inside the string.
                rsts.replace(QLatin1Char('\''), QLatin1String("'\\''"));
            } else if (rsts.isEmpty() || hasSpecialCharsUnix(rsts)) {
                // String contains "quote-worthy" characters. Use single quoting - but
                // that choice is arbitrary.
                rsts.replace(QLatin1Char('\''), QLatin1String("'\\''"));
                rsts.prepend(QLatin1Char('\''));
                rsts.append(QLatin1Char('\''));
            }   // Else just use the string verbatim.
            str.replace(pos, varLen, rsts);
            pos += rsts.length();
            varPos = pos;
            if (!(varLen = findMacro(str, &varPos, &rsts)))
                break;
            continue;
        }
        ushort cc = str.unicode()[pos].unicode();
        if (state.current == MxSingleQuote) {
            // Single quoted context - only the single quote has any special meaning.
            if (cc == '\'')
                state = sstack.pop();
        } else if (cc == '\\') {
            // In any other context, the backslash starts an escape.
            pos += 2;
            if (varPos < pos)
                return false;   // Backslash'd quoted expando would be Bad (TM).
            continue;
        } else if (cc == '$') {
            cc = str.unicode()[++pos].unicode();
            if (cc == '(') {
                sstack.push(state);
                if (str.unicode()[pos + 1].unicode() == '(') {
                    // $(( starts a math expression. This may also be a $( ( in fact,
                    // so we push the current string and offset on a stack so we can retry.
                    MxSave sav = { str, pos + 2, varPos };
                    ostack.push(sav);
                    state.current = MxMath;
                    pos += 2;
                    continue;
                } else {
                    // $( starts a command substitution. This actually "opens a new context"
                    // which overrides surrounding double quoting.
                    state.current = MxParen;
                    state.dquote = false;
                }
            } else if (cc == '{') {
                // ${ starts a "braced" variable substitution.
                sstack.push(state);
                state.current = MxSubst;
            }   // Else assume that a "bare" variable substitution has started
        } else if (cc == '`') {
            str.replace(pos, 1, QLatin1String("$( "));   // add space -> avoid creating $((
            varPos += 2;
            int pos2 = pos += 3;
            forever {
                if (pos2 >= str.length())
                    return false;   // Syntax error - unterminated backtick expression.
                cc = str.unicode()[pos2].unicode();
                if (cc == '`')
                    break;
                if (cc == '\\') {
                    cc = str.unicode()[++pos2].unicode();
                    if (cc == '$' || cc == '`' || cc == '\\' || (cc == '"' && state.dquote)) {
                        str.remove(pos2 - 1, 1);
                        if (varPos >= pos2)
                            varPos--;
                        continue;
                    }
                }
                pos2++;
            }
            str[pos2] = QLatin1Char(')');
            sstack.push(state);
            state.current = MxParen;
            state.dquote = false;
            continue;
        } else if (state.current == MxDoubleQuote) {
            // (Truly) double quoted context - only remaining special char is the closing quote.
            if (cc == '"')
                state = sstack.pop();
        } else if (cc == '\'') {
            // Start single quote if we are not in "inherited" double quoted context.
            if (!state.dquote) {
                sstack.push(state);
                state.current = MxSingleQuote;
            }
        } else if (cc == '"') {
            // Same for double quoting.
            if (!state.dquote) {
                sstack.push(state);
                state.current = MxDoubleQuote;
                state.dquote = true;
            }
        } else if (state.current == MxSubst) {
            // "Braced" substitution context - only remaining special char is the closing brace.
            if (cc == '}')
                state = sstack.pop();
        } else if (cc == ')') {
            if (state.current == MxMath) {
                if (str.unicode()[pos + 1].unicode() == ')') {
                    state = sstack.pop();
                    pos += 2;
                } else {
                    // False hit: the $(( was a $( ( in fact.
                    // ash does not care (and will complain), but bash actually parses it.
                    varPos = ostack.top().varPos;
                    pos = ostack.top().pos;
                    str = ostack.top().str;
                    ostack.pop();
                    state.current = MxParen;
                    state.dquote = false;
                    sstack.push(state);
                }
                continue;
            } else if (state.current == MxParen) {
                state = sstack.pop();
            } else {
                break;   // Syntax error - excess closing parenthesis.
            }
        } else if (cc == '}') {
            if (state.current == MxGroup)
                state = sstack.pop();
            else
                break;   // Syntax error - excess closing brace.
        } else if (cc == '(') {
            // Context-saving command grouping.
            sstack.push(state);
            state.current = MxParen;
        } else if (cc == '{') {
            // Plain command grouping.
            sstack.push(state);
            state.current = MxGroup;
        }
        pos++;
    }

    *cmd = str;
    return true;
}

MacroExpander::MacroExpander(QObject *parent)
    : QObject(parent),
      d(new MacroExpanderPrivate(this))
{
}

MacroExpander::~MacroExpander()
{
    delete d;
}

QString MacroExpander::value(const QString &variable, bool *found)
{
    auto info = d->macroMap.value(variable);
    if (info.cb) {
        if (found) *found = true;
        return info.cb();
    }

    if (found) *found = false;
    return {};
}

QString MacroExpander::expand(const QString &unexpandVariable) const
{
    QString str = unexpandVariable;
    QString ret;
    for (int pos = 0; int len = d->findMacro(str, &pos, &ret);) {
        str.replace(pos, len, ret);
        pos += ret.length();
    }

    return str;
}

QString MacroExpander::expandArguments(const QString &args) const
{
    QString result = args;
    d->expandMacros(&result);
    return result;
}

void MacroExpander::registerVariable(const QString &variable, const QString &description, const StringFunction &value)
{
    MacroExpanderPrivate::MacroInfo info;
    info.description = description;
    info.cb = value;

    d->macroMap.insert(variable, info);
}

void MacroExpander::registerFileVariables(const QString &prefix, const QString &heading, const FileFunction &base)
{
    registerVariable(
            prefix + FilePathPostfix,
            tr("%1: Full path including file name.").arg(heading),
            [base] { return base().absoluteFilePath(); });

    registerVariable(
            prefix + PathPostfix,
            tr("%1: Full path excluding file name.").arg(heading),
            [base] { return base().absolutePath(); });

    registerVariable(
            prefix + FileNamePostfix,
            tr("%1: File name without path.").arg(heading),
            [base] { return base().fileName(); });

    registerVariable(
            prefix + FileBaseNamePostfix,
            tr("%1: File base name without path and suffix.").arg(heading),
            [base] { return base().baseName(); });
}

QStringList MacroExpander::variables() const
{
    return d->macroMap.keys();
}

QString MacroExpander::description(const QString &variable) const
{
    return d->macroMap.value(variable).description;
}

QString MacroExpander::displayName() const
{
    return d->displayName;
}

void MacroExpander::setDisplayName(const QString &displayName)
{
    d->displayName = displayName;
}

class GlobalMacroExpander : public MacroExpander
{
public:
    GlobalMacroExpander()
    {
        setDisplayName(QObject::tr("Global variables"));
    }
};

MacroExpander *globalMacroExpander()
{
    static GlobalMacroExpander ins;
    return &ins;
}
