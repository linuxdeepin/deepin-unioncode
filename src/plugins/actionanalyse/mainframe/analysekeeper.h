// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
