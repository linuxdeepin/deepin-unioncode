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
#ifndef ANALYSEKEEPER_H
#define ANALYSEKEEPER_H

#include "wordcountanalyse.h"

#include <QObject>

class AnalyseKeeper final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AnalyseKeeper)
public:
    AnalyseKeeper();
    static AnalyseKeeper *instance();

public slots:
    void doAnalyse(const ActionAnalyseArgs &args);

private slots:
    void doAnalyseDone(bool result);
};

#endif // ANALYSEKEEPER_H
