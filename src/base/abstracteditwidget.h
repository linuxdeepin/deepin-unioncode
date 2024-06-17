// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTEDITWIDGET_H
#define ABSTRACTEDITWIDGET_H

#include <QWidget>

class AbstractEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractEditWidget(QWidget* parent = nullptr);
    virtual ~AbstractEditWidget() = default;  // Virtual destructor

    // Use virtual for methods that are intended to be overridden in derived classes.
    virtual void closeWidget() = 0; 

signals:
    // You can add custom signals here to notify other objects of events
    // For example: void textChanged(const QString& newText);
};

#endif  // ABSTRACTEDITWIDGET_H

