// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTWIDGET_H
#define ABSTRACTWIDGET_H

#include <QWidget>
#include <memory> // For using std::unique_ptr
#include <DWidget>  // Include the DWidget header

DWIDGET_USE_NAMESPACE

class AbstractWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractWidget(DWidget* qWidget); 
    virtual ~AbstractWidget() = default;  // Virtual destructor

    // Getter method for the DWidget pointer (using const)
    DWidget* qWidget() const;  

signals:
    // You can add custom signals here to communicate events from the widget
    // For example: void widgetClosed();

private:
    class AbstractWidgetPrivate; // Forward declare private implementation class
    std::unique_ptr<AbstractWidgetPrivate> d; // Use unique_ptr for memory management
};

#endif // ABSTRACTWIDGET_H

