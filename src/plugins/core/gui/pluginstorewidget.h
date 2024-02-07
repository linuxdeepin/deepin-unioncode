// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef PLUGINSTOREWIDGET_H
#define PLUGINSTOREWIDGET_H

#include "dtkwidget_global.h"

#include <QWidget>

class PluginListView;
DWIDGET_BEGIN_NAMESPACE
class DSearchEdit;
DWIDGET_END_NAMESPACE

class PluginStoreWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PluginStoreWidget(QWidget *parent = nullptr);
    const PluginListView *getPluginListView() const;

signals:

public slots:
    void slotSearchChanged(const QString &searchText);

private:
    void initializeUi();

    PluginListView *pluginListView {nullptr};
    DTK_WIDGET_NAMESPACE::DSearchEdit *inputEdit { nullptr };
};

#endif // PLUGINSTOREWIDGET_H
