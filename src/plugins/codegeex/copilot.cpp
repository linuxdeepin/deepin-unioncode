// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "copilot.h"
#include "services/editor/editorservice.h"
#include "services/option/optionmanager.h"
#include "services/window/windowservice.h"

#include <QMenu>
#include <QDebug>
#include <QTimer>

//static const char *kUrlGenerateOneLine = "https://tianqi.aminer.cn/api/v2/multilingual_code_generate";
static const char *kUrlGenerateMultiLine = "https://tianqi.aminer.cn/api/v2/multilingual_code_generate_adapt";
static const char *kUrlComment = "https://tianqi.aminer.cn/api/v2/multilingual_code_explain";
static const char *kUrlTranslate = "https://tianqi.aminer.cn/api/v2/multilingual_code_translate";
//static const char *kUrlBugfix = "https://tianqi.aminer.cn/api/v2/multilingual_code_bugfix";
// this is a temporary key
static const char *kDefaultApiKey = "f30ea902c3824ee88e221a32363c0823";

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
        case CopilotApi::multilingual_code_explain:
            if (editorService->replaceSelectedText) {
                editorService->replaceSelectedText(response);
            }
            break;
        case CopilotApi::multilingual_code_generate:
            mutexResponse.lock();
            generateResponse = response;
            if (editorService->showTips && responseValid(response)) {
                editorService->showTips(generateResponse);
            }
            mutexResponse.unlock();
            break;
        case CopilotApi::multilingual_code_translate:
            emit translatedResult(response, dstLang);
            break;
        default:;
        }
    });

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

QString Copilot::apiKey() const
{
    return kDefaultApiKey;
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

    QAction *addComment = new QAction(tr("add comment"));
    QAction *translate = new QAction(tr("translate"));
    menu->addAction(addComment);
    menu->addAction(translate);

    connect(addComment, &QAction::triggered, this, &Copilot::addComment);
    connect(translate, &QAction::triggered, this, &Copilot::translate);

    return menu;
}

void Copilot::translateCode(const QString &code, const QString &dstLanguage)
{
    copilotApi.postTranslate(kUrlTranslate, apiKey(), code, "c++", dstLanguage);
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

void Copilot::processKeyPressEvent(Qt::Key key)
{
    mutexResponse.lock();
    if (key == Qt::Key_Tab && !generateResponse.isEmpty()) {
        insterText(generateResponse);
        generateResponse = "";
    }
    mutexResponse.unlock();

    // start generate code.
    QMetaObject::invokeMethod(this, [this]() {
        timer.start(200);
    });
}

void Copilot::addComment()
{
    copilotApi.postComment(kUrlComment,
                           apiKey(),
                           selectedText(),
                           "cpp",
                           "zh-CN");
}

void Copilot::generateCode()
{
    QString prompt = editorService->getCursorBeforeText();
    QString suffix = editorService->getCursorBehindText();

    copilotApi.postGenerate(kUrlGenerateMultiLine,
                            apiKey(),
                            prompt,
                            suffix);
}

void Copilot::login()
{
}

void Copilot::translate()
{
    emit translatingText(selectedText());

    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService->switchWidgetNavigation)
        windowService->switchWidgetNavigation(MWNA_CODEGEEX);
}
