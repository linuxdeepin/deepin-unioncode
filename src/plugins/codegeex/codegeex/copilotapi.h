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

static const QStringList ALL_AST_LANGS = {
    "javascript",
    "typescript",
    "javascript jsx",
    "javascriptreact",
    "typescript jsx",
    "typescriptreact",
    "go",
    "ruby",
    "csharp",
    "c#",
    "c",
    "cpp",
    "c++",
    "java",
    "rust",
    "python",
};

struct InlineChatInfo {
    enum CommandType{
        Programing,
        Chat
    };
    QString fileName { "" };
    QString package_code { "" };   // eg: "10 import path \n11 import os \n15 import pygame"
    QString class_function { "" };   // if is_ast is false, class_function do not be set. or set it to "all class and function definition in current file"
    QString selectedCode { "" };  // need to contains [cursor]
    QString contextCode { "" };  // linenumber before code
    CommandType commandType { Programing };
    bool is_ast { false }; // true when above langs contains this file type and file content > 100 lines
};

struct CommitMessage {
    QString git_diff { "" };
    QString commit_history { "" };
    QString commit_type { "conventional" };  // conventional / auto / default
};

class CopilotApi : public QObject
{
    Q_OBJECT

public:
    enum GenerateType {
        Line,
        Block
    };

    CopilotApi(QObject *parent = nullptr);
    void setModel(languageModel model);
    languageModel model() const;

    void postGenerate(const QString &url, const QString &prefix, const QString &suffix, GenerateType type);

    void postComment(const QString &url,
                     const QString &code,
                     const QString &locale);

    void postInlineChat(const QString &url,
                        const QString &prompt,
                        const InlineChatInfo &info,
                        const QString &locale);  // codegeex: this api is non-streaming. url need to be xxx/?stream=false

    void postCommit(const QString &url,
                    const CommitMessage &message,
                    const QString &locale);

    void postCommand(const QString &url,
                     const QString &code,
                     const QString &locale,
                     const QString &command);

    enum ResponseType {
        inline_completions,
        inline_chat,
        multilingual_code_comment,
        receiving_by_stream
    };

signals:
    void response(ResponseType responseType, const QString &response, const QString &dstLang);
    void responseByStream(const QString &msgID, const QString &response, const QString &event);
    void asyncGenerateMessages(const QString &url, const QByteArray &body);

    void requestStop();
    void messageSended();

public slots:
    void slotReadReply(QNetworkReply *reply);
    void slotReadReplyStream(QNetworkReply *reply);
    void slotPostGenerateMessage(const QString &url, const QByteArray &body);

private:
    QNetworkReply *postMessage(const QString &url, const QString &token, const QByteArray &body);

    QByteArray assembleGenerateBody(const QString &prefix,
                                    const QString &suffix,
                                    GenerateType type);

    QByteArray assembleInlineChatBody(const QString &prompt,
                                     const InlineChatInfo &info,
                                     const QString &locale);

    QByteArray assembleCommitBody(const CommitMessage &message,
                                  const QString &locale);

    QByteArray assembleCommandBody(const QString &code,
                                   const QString &locale,
                                   const QString &command);

    void processResponse(QNetworkReply *reply);
    QPair<QString, QString> getCurrentFileInfo();

    QNetworkAccessManager *manager = nullptr;
    QString chatModel = chatModelLite;
    QString completionModel = completionModelLite;

    QNetworkReply *completionReply = nullptr;
};

}   // end namespace
Q_DECLARE_METATYPE(CodeGeeX::CopilotApi::ResponseType)

#endif   // COPILOT_H
