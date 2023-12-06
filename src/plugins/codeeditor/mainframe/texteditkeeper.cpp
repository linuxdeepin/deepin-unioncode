// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditkeeper.h"

TextEdit *TextEditKeeper::create(const QString &language, QString *err)
{
    TextEdit *textEdit = instance()->editFactory.create(language, err);
    connect(textEdit, &TextEdit::focusChanged, [=](bool focused){
        if (focused)
            instance()->activeTextEdit = textEdit;
    });
    connect(textEdit, &TextEdit::fileClosed, [=](const QString &file){
        Q_UNUSED(file)
        instance()->activeTextEdit = nullptr;
    });
    return textEdit;
}

void TextEditKeeper::setAnalysedLanguage(const QString &lang)
{
    instance()->analysedLanguage = lang;
}

QString TextEditKeeper::getAnalysedLanguage()
{
    return instance()->analysedLanguage;
}

void TextEditKeeper::setAnalysedWorkspace(const QString &workspace)
{
    instance()->analysedWorkspace = workspace;
}

QString TextEditKeeper::getAnalysedWorkspace()
{
    return instance()->analysedWorkspace;
}

void TextEditKeeper::setAnalysedStorage(const QString &storage)
{
    instance()->analysedStorage = storage;
}

QString TextEditKeeper::getAnalysedStorage()
{
    return instance()->analysedStorage;
}

void TextEditKeeper::setAnalysedData(const AnalysedData &data)
{
    instance()->data = data;
}

void TextEditKeeper::cleanAnalysedData()
{
    instance()->data.rules.clear();
    instance()->data.tokenMaps.clear();
}

QString TextEditKeeper::userActionAnalyseTitle()
{
    return "User Action Analyse"; // can't translation this, crashed
}

QString TextEditKeeper::getTokenTypeAnnLine(const QString &tokenType, const QString &displayText)
{
    QString result;
    auto data = instance()->data;
    auto rules = data.rules;
    for(auto tokenMap: data.tokenMaps) {
        if (tokenType.toStdString() != tokenMap.semanticTokenType
                || tokenMap.result.empty())
            continue;
        for (size_t idx = 0; idx < rules.size(); idx++) {
            if (!result.isEmpty()) result += " ";
            result += QString::fromStdString(data.rules[idx]);
            if (!result.isEmpty()) result += ": ";
            result += QString::number(tokenMap.result[idx]*100) + "%";
        }
    }
    if (!result.isEmpty()) {
        QString textInfo;
        if (!displayText.isEmpty()) {
            textInfo += "\"" + displayText + "\"";
        }
        if (!textInfo.isEmpty())
            result = tokenType + " " + textInfo + " " + result;
    }
    return result;
}

AnalysedData TextEditKeeper::analysedData()
{
    return instance()->data;
}

dpfservice::ProjectInfo TextEditKeeper::projectInfo()
{
    return instance()->proInfo;
}

void TextEditKeeper::saveProjectInfo(const dpfservice::ProjectInfo &info)
{
    instance()->proInfo = info;
}

void TextEditKeeper::removeProjectInfo(const dpfservice::ProjectInfo &info)
{
    if (instance()->proInfo.workspaceFolder() == info.workspaceFolder()) {
        instance()->proInfo = dpfservice::ProjectInfo();
    }
}

TextEdit *TextEditKeeper::getActiveTextEdit()
{
    return instance()->activeTextEdit;
}
