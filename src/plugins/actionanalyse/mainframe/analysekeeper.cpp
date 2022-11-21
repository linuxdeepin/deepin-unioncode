/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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
#include "analysekeeper.h"

#include "common/common.h"

#include <QStandardItem>

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
        ActionAnalyseArgs args = wordcounter->args();
        ContextDialog::ok(QDialog::tr(
                              "Error parsing project symbol\n"
                              "workspace: %0\n"
                              "language: %1\n"
                              "storage: %2\n")
                          .arg(args.workspace)
                          .arg(args.language)
                          .arg(args.storage));
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

    actionanalyse.analyseDone({analyseArgs.workspace, analyseArgs.language,
                               analyseArgs.storage, QVariant::fromValue(analyseData)});
}
