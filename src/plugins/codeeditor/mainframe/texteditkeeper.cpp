/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "texteditkeeper.h"

TextEdit *TextEditKeeper::create(const QString &language, QString *err)
{
    return instance()->editFactory.create(language, err);
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
