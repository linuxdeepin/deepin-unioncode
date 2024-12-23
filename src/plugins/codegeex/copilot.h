// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef COPILOT_H
#define COPILOT_H

#include "services/ai/aiservice.h"
#include "codegeexmanager.h"

#include <QObject>
#include <QTimer>
#include <QMutex>

class QMenu;
namespace dpfservice {
class EditorService;
}

namespace CodeGeeX {
class CodeGeeXCompletionProvider;
}
class InlineChatWidget;
class Command;
class Copilot : public QObject
{
    Q_OBJECT
public:
    static Copilot *instance();
    QMenu *getMenu();

    void replaceSelectedText(const QString &text);
    void insterText(const QString &text);

    void setLocale(CodeGeeX::Locale locale);
    void setCommitsLocale(CodeGeeX::Locale locale);
    void handleSelectionChanged(const QString &fileName, int lineFrom, int indexFrom,
                                int lineTo, int indexTo);
    void handleInlineWidgetClosed();
    void setCopilotLLM(AbstractLLM *llm);

signals:
    void messageSended();
    void requestStop();

public slots:
    void addComment();
    void fixBug();
    void explain();
    void review();
    void tests();
    void commits();

private:
    explicit Copilot(QObject *parent = nullptr);
    QString selectedText() const;
    CodeGeeX::Locale locale { CodeGeeX::Zh };
    CodeGeeX::Locale commitsLocale { CodeGeeX::En };
    void switchToCodegeexPage();
    QString assembleCodeByCurrentFile(const QString &code);
    void showLineChatTip(const QString &fileName, int line);
    void startInlineChat();

    InlineChatWidget *inlineChatWidget = nullptr;
    Command *lineChatCmd = nullptr;
    dpfservice::EditorService *editorService = nullptr;

    AbstractLLM *copilotLLM = nullptr;
};

#endif   // COPILOT_H
