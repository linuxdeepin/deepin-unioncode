// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "copilot.h"
#include "services/editor/editorservice.h"
#include "services/option/optionmanager.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"

#include <QMenu>
#include <QDebug>
#include <QTimer>

static const char *kUrlSSEChat = "https://codegeex.cn/prod/code/chatCodeSseV3/chat";
static const char *kUrlGenerateMultiLine = "https://api.codegeex.cn:8443/tx/v3/completions/inline?stream=false";

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

    connect(&copilotApi, &CopilotApi::response, [this](CopilotApi::ResponseType responseType, const QString &response, const QString &dstLang) {
        switch (responseType) {
        case CopilotApi::multilingual_code_comment:
            replaceSelectedText(response);
            break;
        case CopilotApi::inline_completions:
            mutexResponse.lock();
            generateResponse = response;
            if (editorService->setCompletion && responseValid(response)) {
                editorService->setCompletion(generateResponse, QIcon::fromTheme("codegeex_anwser_icon"), QKeySequence(Qt::CTRL | Qt::Key_T));
            }
            mutexResponse.unlock();
            break;
        case CopilotApi::multilingual_code_translate:
            emit translatedResult(response, dstLang);
            break;
        default:;
        }
    });

    connect(&copilotApi, &CopilotApi::responseByStream, this, &Copilot::response);
    connect(&copilotApi, &CopilotApi::messageSended, this, &Copilot::messageSended);

    timer.setSingleShot(true);

    connect(&timer, &QTimer::timeout, [this]() {
        generateCode();
    });
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
    QString url = QString(kUrlSSEChat) + "?stream=false"; //receive all msg at once
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
    generateCodeEnabled = enabled;
}

void Copilot::setLocale(const QString &locale)
{
    this->locale = locale;
}

void Copilot::setCurrentModel(CodeGeeX::languageModel model)
{
    copilotApi.setModel(model);
}

void Copilot::handleTextChanged()
{
    // start generate code.
    QMetaObject::invokeMethod(this, [this]() {
        timer.start(200);
    });
}

void Copilot::addComment()
{
    QString url = QString(kUrlSSEChat) + "?stream=false"; //receive all msg at once
    copilotApi.postComment(url,
                           selectedText(),
                           locale);
}

void Copilot::generateCode()
{
    if (!generateCodeEnabled)
        return;

    QString prompt = editorService->getCursorBeforeText();
    QString suffix = editorService->getCursorBehindText();

    copilotApi.postGenerate(kUrlGenerateMultiLine,
                            prompt,
                            suffix);
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
    copilotApi.postCommand(url, selectedText(), locale, commandFixBug);

    switchToCodegeexPage();
}

void Copilot::explain()
{
    QString url = QString(kUrlSSEChat) + "?stream=true";
    copilotApi.postCommand(url, selectedText(), locale, commandExplain);

    switchToCodegeexPage();
}

void Copilot::review()
{
    QString url = QString(kUrlSSEChat) + "?stream=true";
    copilotApi.postCommand(url, selectedText(), locale, commandReview);

    switchToCodegeexPage();
}

void Copilot::tests()
{
    QString url = QString(kUrlSSEChat) + "?stream=true";
    copilotApi.postCommand(url, selectedText(), locale, commandTests);

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

    connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, &process](int exitCode, QProcess::ExitStatus exitStatus){
        Q_UNUSED(exitStatus)

        if (exitCode != 0)
            return;

        auto diff = QString::fromUtf8(process.readAll());
        QString url = QString(kUrlSSEChat) + "?stream=true";

        copilotApi.postCommand(url, diff, locale, commandCommits);
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
