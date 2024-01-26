//// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
////
//// SPDX-License-Identifier: GPL-3.0-or-later

//#ifndef TEXTEDITKEEPER_H
//#define TEXTEDITKEEPER_H

//#include "textedittabwidget/textedit.h"
//#include "common/project/projectinfo.h"
//#include <framework/service/qtclassfactory.h>
//#include <framework/service/qtclassmanager.h>

//class TextEditKeeper : public QObject
//{
//    Q_OBJECT

//    dpf::QtClassFactory<TextEdit> editFactory;
//    QString analysedLanguage;
//    QString analysedWorkspace;
//    QString analysedStorage;
//    AnalysedData data;
//    dpfservice::ProjectInfo proInfo;
//    TextEdit *activeTextEdit = nullptr;
//    TextEditKeeper(){}

//public:
//    inline static TextEditKeeper *instance(){
//        static TextEditKeeper ins;
//        return &ins;
//    }

//    template<class Edit>
//    static bool impl(const QString &language = Edit::implLanguage(), QString *err = nullptr) {
//        bool result = true;
//        result &= instance()->editFactory.regClass<Edit>(language, err);
//        return result;
//    }

//    static TextEdit *create(const QString &language, QString *err = nullptr);

//    static void setAnalysedLanguage(const QString &lang);
//    static QString getAnalysedLanguage();

//    static void setAnalysedWorkspace(const QString &workspace);
//    static QString getAnalysedWorkspace();

//    static void setAnalysedStorage(const QString &storage);
//    static QString getAnalysedStorage();

//    static void setAnalysedData(const AnalysedData &data);
//    static AnalysedData analysedData();
//    static void cleanAnalysedData();

//    static QString userActionAnalyseTitle();
//    static QString getTokenTypeAnnLine(const QString &tokenType, const QString &displayText);

//    static dpfservice::ProjectInfo projectInfo();
//    static void saveProjectInfo(const dpfservice::ProjectInfo &info);
//    static void removeProjectInfo(const dpfservice::ProjectInfo &info);

//    static TextEdit *getActiveTextEdit();
//};

//#endif // TEXTEDITKEEPER_H
