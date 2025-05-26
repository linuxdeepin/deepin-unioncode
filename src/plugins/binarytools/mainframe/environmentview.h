// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENVIRONMENTVIEW_H
#define ENVIRONMENTVIEW_H

#include <QAbstractTableModel>
#include <QWidget>

class EnvironmentViewPrivate;
class EnvironmentView : public QWidget
{
    Q_OBJECT
public:
    explicit EnvironmentView(QWidget *parent = nullptr);
    ~EnvironmentView();

    const QMap<QString, QVariant> getEnvironment();
    void appendRow();
    void deleteRow();
    void initModel();
    void setValue(const QMap<QString, QVariant> &map);

    static QMap<QString, QVariant> defaultEnvironment();

public slots:
    void disableDleteButton();
    void handleFocusIndex(const QModelIndex &index);

signals:
    void deleteSignal(bool enable);

private:
    EnvironmentViewPrivate *const d;
};

#endif   // ENVIRONMENTVIEW_H
