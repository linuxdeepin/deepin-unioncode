// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <DWidget>

class ProgressBarPrivate;
class ProgressBar : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
    ProgressBarPrivate *const d;

public:
    explicit ProgressBar(QWidget * parent = nullptr);
    virtual ~ProgressBar();
    void setPercentage(int percentage);
    void setColor(const QColor &color);

protected:
    virtual void paintEvent(QPaintEvent *event);
};

#endif // PROGRESSBAR_H
