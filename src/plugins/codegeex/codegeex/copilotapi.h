// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef COPILOTAPI_H
#define COPILOTAPI_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

namespace CodeGeeX {

static QList<QString> SupportLanguage {
    "python",
    "go",
    "java",
    "javascript",
    "c++",
    "c#",
    "php",
    "typescript",
    "c",
    "css",
    "cuda",
    "dart",
    "lua",
    "objective-c",
    "objective-c++",
    "perl",
    "prolog",
    "swift",
    "lisp",
    "scala",
    "tex",
    "rust",
    "markdown",
    "html",
    "vue",
    "shell",
    "sql",
    "kotlin",
    "visual basic",
    "ruby",
    "pascal",
    "r",
    "fortran",
    "lean",
    "matlab",
    "delphi",
    "scheme",
    "basic",
    "assembly",
    "groovy",
    "abap",
    "gdscript",
    "haskell",
    "julia",
    "elixir",
    "excel",
    "clojure",
    "actionscript",
    "solidity",
    "powershell",
    "erlang",
    "cobol",
    "alloy",
    "awk",
    "thrift",
    "sparql",
    "augeas",
    "f#",
    "cmake",
    "stan",
    "isabelle",
    "dockerfile",
    "rmarkdown",
    "literate agda",
    "glsl",
    "antlr",
    "verilog",
    "racket",
    "standard ml",
    "elm",
    "yaml",
    "smalltalk",
    "ocaml",
    "idris",
    "protocal buffer",
    "bluespec",
    "applescript",
    "makefile",
    "maple",
    "tcsh",
    "systemverilog",
    "literate coffeescript",
    "vhdl",
    "restructuredtext",
    "sas",
    "literate haskell",
    "java server pages",
    "coffeescript",
    "emacs lisp",
    "mathematica",
    "xslt",
    "ada",
    "zig",
    "common lisp",
    "staga",
    "agda",
};

class CopilotApi : public QObject
{
    Q_OBJECT

public:
    CopilotApi(QObject *parent = nullptr);

    void postGenerate(const QString &url, const QString &apiKey, const QString &prompt, const QString &suffix);

    void postComment(const QString &url,
                     const QString &apiKey,
                     const QString &prompt,
                     const QString &lang,
                     const QString &locale,
                     const QString &apisecret = "");

    void postTranslate(const QString &url,
                       const QString &apiKey,
                       const QString &prompt,
                       const QString &src_lang,
                       const QString &dst_lang,
                       const QString &apisecret = "");

    void postFixBug(const QString &url,
                    const QString &apiKey,
                    const QString &prompt,
                    const QString &lang,
                    const QString &apisecret = "");

    enum ResponseType
    {
        multilingual_code_generate,
        multilingual_code_explain,
        multilingual_code_translate,
        multilingual_code_bugfix
    };

signals:
    void response(ResponseType responseType, const QString &response);

private:
    QNetworkReply *postMessage(const QString &url, const QByteArray &body);

    QByteArray assembleGenerateBody(const QString &prompt,
                             const QString &suffix,
                             const QString &apikey,
                             int n = 1,
                             const QString &apisecret = "");

    QByteArray assembleCommentBody(const QString &prompt,
                             const QString &lang,
                             const QString &locale,
                             const QString &apikey,
                             const QString &apisecret = "");

    QByteArray assembleTranslateBody(const QString &prompt,
                             const QString &src_lang,
                             const QString &dst_lang,
                             const QString &apikey,
                             const QString &apisecret = "");

    QByteArray assembleBugfixBody(const QString &prompt,
                             const QString &lang,
                             const QString &apikey,
                             const QString &apisecret = "");

    void processResponse(QNetworkReply *reply);

    QNetworkAccessManager *manager = nullptr;
};

}// end namespace

#endif // COPILOT_H
