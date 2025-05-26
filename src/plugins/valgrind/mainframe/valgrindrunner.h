// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VALGRINDRUNNER_H
#define VALGRINDRUNNER_H

#include "common/widget/outputpane.h"
#include "services/language/languagegenerator.h"

#include <QObject>

const QString HELGRIND = "helgrind";
const QString MEMCHECK = "memcheck";

class ValgrindRunnerPrivate;
class ValgrindRunner : public QObject
{
    Q_OBJECT
public:
    explicit ValgrindRunner(QObject *parent = nullptr);

    void initialize();
    void runValgrind(const QString &type);
    void saveCurrentProjectInfo(const dpfservice::ProjectInfo &projectInfo);
    void removeProjectInfo();
    void saveCurrentFilePath(const QString &filePath);
    void removeCurrentFilePath();

    static ValgrindRunner *instance();

signals:
    void valgrindFinished(const QString &xmlFilePath, const QString &type);
    void clearValgrindBar(const QString &type);

private slots:
    void printOutput(const QString &content, OutputPane::OutputFormat format);

private:
    void setValgrindArgs(const QString &type);
    void setMemcheckArgs(QStringList &args);
    void setHelgrindArgs(QStringList &args);
    void setActionsStatus(const QString &kitName);
    void runBuilding();
    void outputMsg(const QString &content, OutputPane::OutputFormat format);
    bool checkValgrindToolPath();

    ValgrindRunnerPrivate *const d;
};

#endif // VALGRINDRUNNER_H
