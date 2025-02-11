// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "copilot.h"
#include "base/ai/abstractllm.h"
#include "widgets/inlinechatwidget.h"
#include "services/editor/editorservice.h"
#include "services/option/optionmanager.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "common/actionmanager/actionmanager.h"

#include <QMenu>
#include <QDebug>
#include <QTimer>
#include <QAction>

static const char *lineChatTip = "LineChatTip";

using namespace Chat;
using namespace dpfservice;

Copilot::Copilot(QObject *parent)
    : QObject(parent)
{
    editorService = dpfGetService(EditorService);
    if (!editorService) {
        qFatal("Editor service is null!");
    }

    QAction *lineChatAct = new QAction(tr("Inline Chat"), this);
    lineChatCmd = ActionManager::instance()->registerAction(lineChatAct, "Chat.InlineChat");
    lineChatCmd->setDefaultKeySequence(Qt::CTRL + Qt::Key_T);
    connect(lineChatAct, &QAction::triggered, this, &Copilot::startInlineChat);
}

QString Copilot::selectedText() const
{
    if (!editorService->getSelectedText)
        return "";

    return editorService->getSelectedText();
}

Copilot *Copilot::instance()
{
    static Copilot ins;
    return &ins;
}

QMenu *Copilot::getMenu()
{
    QMenu *menu = new QMenu();
    menu->setTitle("Chat");

    QAction *addComment = new QAction(tr("Add Comment"));
    QAction *fixBug = new QAction(tr("Fix Bug"));
    QAction *explain = new QAction(tr("Explain Code"));
    QAction *review = new QAction(tr("Review Code"));
    QAction *tests = new QAction(tr("Generate Unit Tests"));
    QAction *commits = new QAction(tr("Generate git commits"));

    menu->addAction(addComment);
    menu->addAction(fixBug);
    menu->addAction(explain);
    menu->addAction(review);
    menu->addAction(tests);
    menu->addAction(commits);

    connect(addComment, &QAction::triggered, this, &Copilot::addComment);
    connect(fixBug, &QAction::triggered, this, &Copilot::fixBug);
    connect(explain, &QAction::triggered, this, &Copilot::explain);
    connect(review, &QAction::triggered, this, &Copilot::review);
    connect(tests, &QAction::triggered, this, &Copilot::tests);
    connect(commits, &QAction::triggered, this, &Copilot::commits);

    return menu;
}

void Copilot::replaceSelectedText(const QString &text)
{
    if (editorService->replaceSelectedText)
        editorService->replaceSelectedText(text);
}

void Copilot::insterText(const QString &text)
{
    if (editorService->insertText)
        editorService->insertText(text);
}

void Copilot::setLocale(Chat::Locale locale)
{
    this->locale = locale;
}

void Copilot::setCommitsLocale(Chat::Locale locale)
{
    this->commitsLocale = locale;
}

void Copilot::handleSelectionChanged(const QString &fileName, int lineFrom, int indexFrom, int lineTo, int indexTo)
{
    editorService->clearAllEOLAnnotation(lineChatTip);
    if (lineFrom == -1)
        return;

    Edit::Position pos = editorService->cursorPosition();
    if (pos.line < 0)
        return;

    showLineChatTip(fileName, pos.line);
}

void Copilot::handleInlineWidgetClosed()
{
    if (inlineChatWidget)
        inlineChatWidget->reset();
}

void Copilot::setCopilotLLM(AbstractLLM *llm)
{
    copilotLLM = llm;
}

void Copilot::addComment()
{
    QString prompt = "You're an intelligent programming assistant."
                     " You'll answer any questions you may have about programming, code, or computers, and provide formatted, executable, accurate, and secure code."
                     " Task: Please provide a comment for the input code, including multi-line comments and single-line comments, please be careful not to change the original code, only add comments."
                     " Directly return the code without any Markdown formatting, such as ```, ```cpp, etc. "
                     "\n\ncode: ```%1```";
    copilotLLM->setStream(false);
    copilotLLM->request(prompt.arg(selectedText()), [=](const QString &data, AbstractLLM::ResponseState state){
        if (state == AbstractLLM::ResponseState::Success) {
            if (!data.isEmpty())
                replaceSelectedText(data);
        }
    });
}

void Copilot::fixBug()
{
    auto currentFileText = editorService->fileText(editorService->currentFile());
    currentFileText.replace(selectedText(), "<START EDITING HERE>" + selectedText() + "<STOP EDITING HERE>");
    QString prompt = "code: ```%1```\n\n"
                     "Rewrite the code between <START EDITING HERE> and <STOP EDITING HERE> in this entire code block. The rewrite requirements are: fix any bugs in this code, or do a simple rewrite if there are no errors, without leaving placeholders. Answer only the code between these markers.";
    ChatManager::instance()->requestAsync(prompt.arg(currentFileText));
    switchToChatPage();
}

void Copilot::explain()
{
    QString prompt = "code: ```%1```\n\n"
                     "You are an intelligent programming assistant. You will answer any questions users have about programming, code, and computers, providing well-formatted, executable, accurate, and secure code, and providing detailed explanations when necessary. Task: Please explain the meaning of the input code, including the implementation principle, purpose, and precautions, etc. ";
    if (locale == Zh)
        prompt.append("\nPlease answer by Chineses");
    ChatManager::instance()->requestAsync(prompt.arg(selectedText()));
    switchToChatPage();
}

void Copilot::review()
{
    QString prompt = "code: ```%1```\n\n"
                     "You are an intelligent programming assistant. You will answer any questions users have about programming, code, and computers, providing well-formatted, executable, accurate, and secure code, and providing detailed explanations when necessary. Now, you will start to act as an advanced expert engineer focusing on code review and software security. Please provide at least three global suggestions based on the user’s code";
    if (locale == Zh)
        prompt.append("\nPlease answer by Chineses");
    ChatManager::instance()->requestAsync(prompt.arg(selectedText()));
    switchToChatPage();
}

void Copilot::tests()
{
    QString prompt = "code: ```%1```\n\n"
                     "You need to automatically determine the programming language of the provided code, and write a set of unit test code for it using a popular current unit testing framework. Please ensure that the tests cover the main functionalities and edge cases, and include necessary comments.";
    if (locale == Zh)
        prompt.append("\nPlease answer by Chineses");
    ChatManager::instance()->requestAsync(prompt.arg(selectedText()));
    switchToChatPage();
}

void Copilot::commits()
{
    QProcess process;
    process.setProgram("git");
    process.setArguments(QStringList() << "diff");
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    auto prjInfo = projectService->getActiveProjectInfo();
    auto workingDirectory = prjInfo.workspaceFolder();
    process.setWorkingDirectory(workingDirectory);

    connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, &process](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitStatus)
        if (exitCode != 0)
            return;

        auto diff = QString::fromUtf8(process.read(20000));
        QString prompt = "diff: ```%1```\n\n"
                         "You always analyze the git diff provided, detect changes, and generate succinct yet comprehensive commit messages. for the user step-by-step:\n1. You first parse the git diff to understand the changes made: additions, deletions, modifications, or renaming.\n2. Identify the components or modules that the changes are relating to.\n3. Understand the nature of changes: bug fixes, functionality enhancements, code optimization, documentation, etc.\n4. You highlight the primary updates without neglecting any minor alterations.\n5. Choose a commit type according to the primary updates.\n6. Organize these changes into an accurate, concise and informative commit message less than 20 words.\nYou should only reply a one-line commit message less than 20 words!!!";
        if (commitsLocale == Zh)
            prompt.append("\nPlease answer by Chineses");
        ChatManager::instance()->requestAsync(prompt.arg(diff));
        switchToChatPage();
    });

    process.start();
    process.waitForFinished();
}

void Copilot::switchToChatPage()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    windowService->showWidgetAtRightspace(MWNA_CHAT);
}

QString Copilot::assembleCodeByCurrentFile(const QString &code)
{
    auto filePath = editorService->currentFile();
    auto fileType = support_file::Language::id(filePath);

    QString result;
    result = "```" + fileType + "\n" + code + "```";
    return result;
}

void Copilot::showLineChatTip(const QString &fileName, int line)
{
    auto keySequences = lineChatCmd->keySequences();
    QStringList keyList;
    for (const auto &key : keySequences) {
        if (key.isEmpty())
            continue;
        keyList << key.toString();
    }

    if (!keyList.isEmpty()) {
        QString msg = InlineChatWidget::tr("  Press %1 to inline chat").arg(keyList.join(','));
        editorService->eOLAnnotate(fileName, lineChatTip, msg, line, Edit::TipAnnotation);
    }
}

void Copilot::startInlineChat()
{
    editorService->closeLineWidget();
    editorService->clearAllEOLAnnotation(lineChatTip);
    if (!inlineChatWidget) {
        inlineChatWidget = new InlineChatWidget;
        connect(inlineChatWidget, &InlineChatWidget::destroyed, this, [this] { inlineChatWidget = nullptr; });
    }

    inlineChatWidget->setLLM(copilotLLM);
    inlineChatWidget->start();
}
