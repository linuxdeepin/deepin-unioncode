// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef COPILOTAPI_H
#define COPILOTAPI_H

#include "codegeexmanager.h"

#include <QObject>
#include <QMetaType>

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
    void setModel(languageModel model);

    void postGenerate(const QString &url, const QString &code, const QString &suffix);

    void postComment(const QString &url,
                     const QString &code,
                     const QString &locale);

    void postTranslate(const QString &url,
                       const QString &code,
                       const QString &dst_lang);

    void postCommand(const QString &url,
                     const QString &code,
                     const QString &locale,
                     const QString &command);

    enum ResponseType
    {
        inline_completions,
        multilingual_code_comment,
        multilingual_code_translate,
        receiving_by_stream
    };

signals:
    void response(ResponseType responseType, const QString &response, const QString &dstLang);
    void responseByStream(const QString &msgID, const QString &response, const QString &event);

    void messageSended();

public slots:
    void slotReadReply(QNetworkReply *reply);
    void slotReadReplyStream(QNetworkReply *reply);

private:
    QNetworkReply *postMessage(const QString &url, const QString &token, const QByteArray &body);

    QByteArray assembleGenerateBody(const QString &prefix,
                             const QString &suffix);

    QByteArray assembleTranslateBody(const QString &code,
                             const QString &dst_lang);

    QByteArray assembleCommandBody(const QString &code,
                             const QString &locale,
                             const QString &command);

    void processResponse(QNetworkReply *reply);

    QNetworkAccessManager *manager = nullptr;
    QString chatModel = chatModelLite;
    QString completionModel = completionModelLite;
};

}// end namespace
Q_DECLARE_METATYPE(CodeGeeX::CopilotApi::ResponseType)

#endif // COPILOT_H
