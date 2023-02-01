/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#ifndef RUNNER_H
#define RUNNER_H

#include "event/event.h"
#include "services/language/languagegenerator.h"
#include "debuggerglobals.h"

#include <QObject>
#include <QAction>

class RunnerPrivate;
class Runner : public QObject
{
    Q_OBJECT
public:
    explicit Runner(QObject *parent = nullptr);


signals:
    void sigOutputMsg(const QString &content, OutputFormat format = NormalMessageFormat);

public slots:
    void run();
    void handleEvents(const dpf::Event &event);

private:
    void running();
    bool execCommand(const dpfservice::RunCommandInfo &info);
    void outputMsg(const QString &content, OutputFormat format = NormalMessageFormat);

private:
   RunnerPrivate *const d;
};

#endif // RUNNER_H
