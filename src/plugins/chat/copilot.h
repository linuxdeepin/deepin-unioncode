// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef COPILOT_H
#define COPILOT_H

#include "services/ai/aiservice.h"
#include "chatmanager.h"

#include <QObject>
#include <QTimer>
#include <QMutex>

class QMenu;
namespace dpfservice {
class EditorService;
}

namespace Chat {
class ChatCompletionProvider;
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

    void setLocale(Chat::Locale locale);
    void setCommitsLocale(Chat::Locale locale);
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
    Chat::Locale locale { Chat::Zh };
    Chat::Locale commitsLocale { Chat::En };
    void switchToChatPage();
    QString assembleCodeByCurrentFile(const QString &code);
    void showLineChatTip(const QString &fileName, int line);
    void startInlineChat();

    InlineChatWidget *inlineChatWidget = nullptr;
    Command *lineChatCmd = nullptr;
    dpfservice::EditorService *editorService = nullptr;

    AbstractLLM *copilotLLM = nullptr;
};

#endif   // COPILOT_H
