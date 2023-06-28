// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QWidget>

class ProgressBarPrivate;
class ProgressBar : public QWidget
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
