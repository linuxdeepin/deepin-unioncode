// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef COPILOT_H
#define COPILOT_H

#include "codegeex/copilotapi.h"

#include <QObject>
#include <QTimer>
#include <QMutex>

class QMenu;
namespace dpfservice {
class EditorService;
}

class Copilot : public QObject
{
    Q_OBJECT
public:
    static Copilot *instance();
    QMenu *getMenu();

    void translateCode(const QString &code, const QString &dstLanguage);
    void replaceSelectedText(const QString &text);
    void insterText(const QString &text);
    void setGenerateCodeEnabled(bool enabled);
    void setLocale(const QString &locale);
    void setCommitsLocale(const QString &locale);
    void setCurrentModel(CodeGeeX::languageModel model);
    void handleTextChanged();

signals:
    // the code will be tranlated.
    void translatingText(const QString &text);
    // the result has been tranlated.
    void translatedResult(const QString &result, const QString &dstLang);

    void response(const QString &msgID, const QString &response, const QString &event);
    void messageSended();

public slots:
    void addComment();
    void generateCode();
    void login();
    void translate();
    void fixBug();
    void explain();
    void review();
    void tests();
    void commits();

private:
    explicit Copilot(QObject *parent = nullptr);
    QString selectedText() const;
    QString locale { "zh" };
    QString commitsLocale { "zh" };
    void switchToCodegeexPage();
    bool responseValid(const QString &response);
    QString assembleCodeByCurrentFile(const QString &code);

    CodeGeeX::CopilotApi copilotApi;
    dpfservice::EditorService *editorService = nullptr;
    QString generateResponse;
    QMutex mutexResponse;
    bool generateCodeEnabled = true;
};

#endif   // COPILOT_H
