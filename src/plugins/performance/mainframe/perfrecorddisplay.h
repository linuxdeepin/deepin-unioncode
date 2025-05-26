// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PERFRECORDDISPLAY_H
#define PERFRECORDDISPLAY_H

#include <QWidget>

class PerfRecordDisplayPrivate;
class PerfRecordDisplay : public QWidget
{
    Q_OBJECT
    PerfRecordDisplayPrivate *const d;
public:
    explicit PerfRecordDisplay(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~PerfRecordDisplay();

signals:
    void showWebBrowserGP(bool flag);
};

#endif // PERFRECORDDISPLAY_H
