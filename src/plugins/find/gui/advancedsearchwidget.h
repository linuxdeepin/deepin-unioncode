// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADVANCEDSEARCHWIDGET_H
#define ADVANCEDSEARCHWIDGET_H

#include <QWidget>

class AdvancedSearchWidgetPrivate;
class AdvancedSearchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AdvancedSearchWidget(QWidget *parent = nullptr);
    ~AdvancedSearchWidget();

    void initOperator();
    QString searchText() const;
    void setSearchText(const QString &text);

protected:
    void showEvent(QShowEvent *e) override;

private:
    AdvancedSearchWidgetPrivate *const d;
};

#endif   // ADVANCEDSEARCHWIDGET_H
