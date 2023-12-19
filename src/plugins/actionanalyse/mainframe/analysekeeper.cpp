// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "analysekeeper.h"

#include "common/common.h"

namespace {
static WordCountAnalyse *wordcounter{nullptr};
static ActionAnalyseArgs analyseArgs;
static AnalysedData analyseData;
}

AnalyseKeeper::AnalyseKeeper()
{

}

AnalyseKeeper *AnalyseKeeper::instance()
{
    static AnalyseKeeper ins;
    return &ins;
}

void AnalyseKeeper::doAnalyse(const ActionAnalyseArgs &args)
{
    analyseArgs = args;
    if (wordcounter) {
        QObject::disconnect(wordcounter, &WordCountAnalyse::analyseDone,
                            this, &AnalyseKeeper::doAnalyseDone);
        wordcounter->kill();
        wordcounter->waitForFinished();
        delete wordcounter;
        wordcounter = nullptr;
    }
    wordcounter = new WordCountAnalyse();
    wordcounter->setArgs(args);
    wordcounter->WordCountAnalyse::start();
    QObject::connect(wordcounter, &WordCountAnalyse::analyseDone,
                     this, &AnalyseKeeper::doAnalyseDone);
}

void AnalyseKeeper::doAnalyseDone(bool result)
{
    if (!result) {
//        ActionAnalyseArgs args = wordcounter->args();
//        ContextDialog::ok(QDialog::tr(
//                              "Error Do User Action Anlyse\n"
//                              "workspace: %0\n"
//                              "language: %1\n"
//                              "storage: %2\n")
//                          .arg(args.workspace)
//                          .arg(args.language)
//                          .arg(args.storage));
    }

    QFile wordcountFile(wordcounter->args().storage + QDir::separator() + ".wordcount");
    QTextStream ts(&wordcountFile);
    if (!wordcountFile.open(QIODevice::ReadWrite)) {
        qDebug("fail to open wordcountFile");
    }

    int totalLineNum = 0;
    while (!ts.atEnd()) {
        ts.readLine();
        totalLineNum++;
    }
    ts.seek(0);

    analyseData.rules.resize(4);
    analyseData.rules[0] = "UpperCamel";
    analyseData.rules[1] = "LowerCamel";
    analyseData.rules[2] = "Snake";
    analyseData.rules[3] = "Others";

    analyseData.tokenMaps.resize(totalLineNum);
    int lineNum = 0;
    while (!ts.atEnd() && lineNum < totalLineNum) {

        QStringList line = ts.readLine().split(" ", QString::SkipEmptyParts);
        analyseData.tokenMaps[lineNum].semanticTokenType = line.at(0).toStdString();
        analyseData.tokenMaps[lineNum].semanticTokenModifier = line.at(1).toStdString();
        analyseData.tokenMaps[lineNum].result.resize(4);
        analyseData.tokenMaps[lineNum].result[0] = line.at(2).toFloat();
        std::cerr << analyseData.tokenMaps[lineNum].result[0];
        analyseData.tokenMaps[lineNum].result[1] = line.at(3).toFloat();
        analyseData.tokenMaps[lineNum].result[2] = line.at(4).toFloat();
        analyseData.tokenMaps[lineNum].result[3] = line.at(5).toFloat();
        lineNum++;
    }

    wordcounter->kill();
    delete wordcounter;
    wordcounter = nullptr;

    actionanalyse.analyseDone(analyseArgs.workspace, analyseArgs.language,
                              analyseArgs.storage, analyseData);
}
