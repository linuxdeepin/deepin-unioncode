/*
 * Copyright (C) 2020 ~ 2023 Uniontech Software Technology Co., Ltd.
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
