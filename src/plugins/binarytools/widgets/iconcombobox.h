// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONCOMBOBOX_H
#define ICONCOMBOBOX_H

#include <QComboBox>
#include <QListView>
#include <QStandardItemModel>

class IconComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit IconComboBox(QWidget *parent = nullptr);

    void showPopup() override;
    void setIcon(const QString &icon);
    QString icon();

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    void initUI();
    void initData();
    void initConnections();

    QFrame *iconFrame { nullptr };
    QListView *iconView { nullptr };
    QStandardItemModel iconModel;
    QString iconName;
};

#endif   // ICONCOMBOBOX_H
