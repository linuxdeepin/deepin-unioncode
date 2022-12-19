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
#ifndef TEXTEDITKEEPER_H
#define TEXTEDITKEEPER_H

#include "textedittabwidget/textedit.h"
#include "services/project/projectinfo.h"
#include <framework/service/qtclassfactory.h>
#include <framework/service/qtclassmanager.h>

class TextEditKeeper final
{
    dpf::QtClassFactory<TextEdit> editFactory;
    QString analysedLanguage;
    QString analysedWorkspace;
    QString analysedStorage;
    AnalysedData data;
    dpfservice::ProjectInfo proInfo;
    TextEditKeeper(){}

    inline static TextEditKeeper *instance(){
        static TextEditKeeper ins;
        return &ins;
    }

public:

    template<class Edit>
    static bool impl(const QString &language = Edit::implLanguage(), QString *err = nullptr) {
        bool result = true;
        result &= instance()->editFactory.regClass<Edit>(language, err);
        return result;
    }

    static TextEdit *create(const QString &language, QString *err = nullptr);

    static void setAnalysedLanguage(const QString &lang);
    static QString getAnalysedLanguage();

    static void setAnalysedWorkspace(const QString &workspace);
    static QString getAnalysedWorkspace();

    static void setAnalysedStorage(const QString &storage);
    static QString getAnalysedStorage();

    static void setAnalysedData(const AnalysedData &data);
    static AnalysedData analysedData();
    static void cleanAnalysedData();

    static QString userActionAnalyseTitle();
    static QString getTokenTypeAnnLine(const QString &tokenType, const QString &displayText);

    static dpfservice::ProjectInfo projectInfo();
    static void saveProjectInfo(const dpfservice::ProjectInfo &info);
    static void removeProjectInfo(const dpfservice::ProjectInfo &info);
};

#endif // TEXTEDITKEEPER_H
