// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEPORTINGWIDGET_H
#define CODEPORTINGWIDGET_H

#include "reportpane.h"
#include "common/widget/outputpane.h"

#include <DFrame>

inline const QString REPORT_SRCLIST { QObject::tr("Source files to migrate") };
inline const QString REPORT_LIBLIST { QObject::tr("Architecture-dependent library files") };

class CodePortingWidget : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit CodePortingWidget(QWidget *parent = nullptr);

private:
    void initUI();

    OutputPane *outputPane {nullptr};
    ReportPane *reportPane {nullptr};
};

#endif // CODEPORTINGWIDGET_H
