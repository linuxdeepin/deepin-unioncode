// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "copilot.h"
#include "widgets/inlinechatwidget.h"
#include "services/editor/editorservice.h"
#include "services/option/optionmanager.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "common/actionmanager/actionmanager.h"

#include <QMenu>
#include <QDebug>
#include <QTimer>

static const char *kUrlSSEChat = "https://codegeex.cn/prod/code/chatCodeSseV3/chat";
static const char *kUrlGenerateMultiLine = "https://api.codegeex.cn:8443/v3/completions/inline?stream=false";

static const char *lineChatTip = "LineChatTip";
static const char *commandFixBug = "fixbug";
static const char *commandExplain = "explain";
static const char *commandReview = "review";
static const char *commandTests = "tests";
static const char *commandCommits = "commit_message";

using namespace CodeGeeX;
using namespace dpfservice;

Copilot::Copilot(QObject *parent)
    : QObject(parent)
{
    editorService = dpfGetService(EditorService);
    if (!editorService) {
        qFatal("Editor service is null!");
    }
    generateTimer = new QTimer(this);
    generateTimer->setSingleShot(true);

    QAction *lineChatAct = new QAction(tr("Inline Chat"), this);
    lineChatCmd = ActionManager::instance()->registerAction(lineChatAct, "CodeGeeX.InlineChat");
    lineChatCmd->setDefaultKeySequence(Qt::CTRL + Qt::Key_T);
    connect(lineChatAct, &QAction::triggered, this, &Copilot::startInlineChat);

    connect(&copilotApi, &CopilotApi::response, [this](CopilotApi::ResponseType responseType, const QString &response, const QString &dstLang) {
        switch (responseType) {
        case CopilotApi::multilingual_code_comment:
            replaceSelectedText(response);
            break;
        case CopilotApi::inline_completions:
            if (!responseValid(response))
                return;
            {
                QString completion = "";

                if (generateType == CopilotApi::Line) {
                    generateCache = response.split('\n');
                    completion = extractSingleLine();
                } else if (generateType == CopilotApi::Block) {
                    generateCache.clear();
                    completion = response;
                }

                if (editorService->setCompletion) {
                    editorService->setCompletion(completion);
                    generatedCode = completion;
                }
            }
            break;
        case CopilotApi::multilingual_code_translate:
            emit translatedResult(response, dstLang);
            break;
        default:;
        }
    });

    connect(&copilotApi, &CopilotApi::responseByStream, this, &Copilot::response);
    connect(&copilotApi, &CopilotApi::messageSended, this, &Copilot::messageSended);
    connect(generateTimer, &QTimer::timeout, this, &Copilot::generateCode);
}

QString Copilot::selectedText() const
{
    if (!editorService->getSelectedText)
        return "";

    return editorService->getSelectedText();
}

bool Copilot::responseValid(const QString &response)
{
    bool valid = !(response.isEmpty()
                   || response.startsWith("\n\n\n")
                   || response.startsWith("\n    \n    "));
    if (!valid) {
        qWarning() << "Reponse not valid: " << response;
    }
    return valid;
}

Copilot *Copilot::instance()
{
    static Copilot ins;
    return &ins;
}

QMenu *Copilot::getMenu()
{
    QMenu *menu = new QMenu();
    menu->setTitle("CodeGeeX");

    QAction *addComment = new QAction(tr("Add Comment"));
    QAction *translate = new QAction(tr("Translate"));
    QAction *fixBug = new QAction(tr("Fix Bug"));
    QAction *explain = new QAction(tr("Explain Code"));
    QAction *review = new QAction(tr("Review Code"));
    QAction *tests = new QAction(tr("Generate Unit Tests"));
    QAction *commits = new QAction(tr("Generate git commits"));

    menu->addAction(addComment);
    menu->addAction(translate);
    menu->addAction(fixBug);
    menu->addAction(explain);
    menu->addAction(review);
    menu->addAction(tests);
    menu->addAction(commits);

    connect(addComment, &QAction::triggered, this, &Copilot::addComment);
    connect(translate, &QAction::triggered, this, &Copilot::translate);
    connect(fixBug, &QAction::triggered, this, &Copilot::fixBug);
    connect(explain, &QAction::triggered, this, &Copilot::explain);
    connect(review, &QAction::triggered, this, &Copilot::review);
    connect(tests, &QAction::triggered, this, &Copilot::tests);
    connect(commits, &QAction::triggered, this, &Copilot::commits);

    return menu;
}

void Copilot::translateCode(const QString &code, const QString &dstLanguage)
{
    QString url = QString(kUrlSSEChat) + "?stream=false";   //receive all msg at once
    copilotApi.postTranslate(url, code, dstLanguage, locale);
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

void Copilot::setGenerateCodeEnabled(bool enabled)
{
    if (!enabled && generateTimer->isActive())
        generateTimer->stop();
    generateCodeEnabled = enabled;
}

bool Copilot::getGenerateCodeEnabled() const
{
    return generateCodeEnabled;
}

void Copilot::setLocale(const QString &locale)
{
    this->locale = locale;
}

void Copilot::setCommitsLocale(const QString &locale)
{
    this->commitsLocale = locale;
}

void Copilot::setCurrentModel(CodeGeeX::languageModel model)
{
    copilotApi.setModel(model);
}

void Copilot::handleTextChanged()
{
    if (!generateCodeEnabled)
        return;

    editorService->setCompletion("");
    QMetaObject::invokeMethod(this, [this]() {
        generateTimer->start(500);
    });
}

void Copilot::handleSelectionChanged(const QString &fileName, int lineFrom, int indexFrom, int lineTo, int indexTo)
{
    QMetaObject::invokeMethod(this, [=] {
        if (!CodeGeeXManager::instance()->isLoggedIn())
            return;

        editorService->clearAllEOLAnnotation(lineChatTip);
        Edit::Position pos = editorService->cursorPosition();
        if (pos.line < 0)
            return;

        if (lineFrom == -1) {
            auto text = editorService->lineText(fileName, pos.line);
            if (!text.remove(QRegExp("\\s+")).isEmpty())
                return;
        }

        showLineChatTip(fileName, pos.line);
    });
}

void Copilot::addComment()
{
    QString url = QString(kUrlSSEChat) + "?stream=false";   //receive all msg at once
    copilotApi.postComment(url,
                           selectedText(),
                           locale);
}

void Copilot::generateCode()
{
    if (!generateCodeEnabled)
        return;

    QString prefix = editorService->getCursorBeforeText();
    QString suffix = editorService->getCursorBehindText();
    if (!prefix.endsWith(generatedCode) || generateCache.isEmpty()) {
        generateType = checkPrefixType(prefix);
        copilotApi.postGenerate(kUrlGenerateMultiLine,
                                prefix,
                                suffix,
                                generateType);
    } else {
        QString completion = extractSingleLine();
        if (editorService->setCompletion) {
            editorService->setCompletion(completion);
            generatedCode = completion;
        }
    }
}

void Copilot::login()
{
}

void Copilot::translate()
{
    emit translatingText(selectedText());
    switchToCodegeexPage();
}

void Copilot::fixBug()
{
    QString url = QString(kUrlSSEChat) + "?stream=true";
    copilotApi.postCommand(url, assembleCodeByCurrentFile(selectedText()), locale, commandFixBug);

    switchToCodegeexPage();
}

void Copilot::explain()
{
    QString url = QString(kUrlSSEChat) + "?stream=true";
    copilotApi.postCommand(url, assembleCodeByCurrentFile(selectedText()), locale, commandExplain);

    switchToCodegeexPage();
}

void Copilot::review()
{
    QString url = QString(kUrlSSEChat) + "?stream=true";
    copilotApi.postCommand(url, assembleCodeByCurrentFile(selectedText()), locale, commandReview);

    switchToCodegeexPage();
}

void Copilot::tests()
{
    QString url = QString(kUrlSSEChat) + "?stream=true";
    copilotApi.postCommand(url, assembleCodeByCurrentFile(selectedText()), locale, commandTests);

    switchToCodegeexPage();
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

        auto diff = QString::fromUtf8(process.readAll());
        QString url = QString(kUrlSSEChat) + "?stream=true";

        copilotApi.postCommand(url, diff, commitsLocale, commandCommits);
        switchToCodegeexPage();
    });

    process.start();
    process.waitForFinished();
}

void Copilot::switchToCodegeexPage()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService->switchWidgetNavigation)
        windowService->switchWidgetNavigation(MWNA_CODEGEEX);
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
    if (!CodeGeeXManager::instance()->isLoggedIn())
        return;

    editorService->clearAllEOLAnnotation(lineChatTip);
    if (!inlineChatWidget) {
        inlineChatWidget = new InlineChatWidget;
        connect(inlineChatWidget, &InlineChatWidget::destroyed, this, [this] { inlineChatWidget = nullptr; });
    }

    inlineChatWidget->start();
}

CodeGeeX::CopilotApi::GenerateType Copilot::checkPrefixType(const QString &prefixCode)
{
    //todo
    Q_UNUSED(prefixCode)
    if (0)
        return CopilotApi::Line;
    else
        return CopilotApi::Block;
}

QString Copilot::extractSingleLine()
{
    if (generateCache.isEmpty())
        return "";

    bool extractedCode = false;
    QString completion = "";
    for (auto line : generateCache) {
        if (extractedCode)
            break;
        if (line != "")
            extractedCode = true;

        completion += line == "" ? "\n" : line;
        generateCache.removeFirst();
    }
    completion += "\n";

    //check if left cache all '\n'
    bool leftAllEmpty = true;
    for (auto line : generateCache) {
        if (line == "")
            continue;
        leftAllEmpty = false;
        break;
    }
    if (leftAllEmpty) {
        generateCache.clear();
        completion += "\n";
    }

    if (!extractedCode)
        completion = "";
    return completion;
}
