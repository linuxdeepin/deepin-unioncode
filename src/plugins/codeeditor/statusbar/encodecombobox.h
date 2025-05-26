// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENCODECOMBOBOX_H
#define ENCODECOMBOBOX_H

#include <DToolButton>

#include <QMenu>
#include <QFrame>

class EncodeComboBox : public QFrame
{
    Q_OBJECT
public:
    explicit EncodeComboBox(QWidget *parent = nullptr);

    void setEncodeName(const QString &encoding);
    QString encodeName() const;

Q_SIGNALS:
    void encodeChanged(const QString &encode);

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    void initUI();
    void initConnection();
    void initMenuData();
    QIcon createIcon();
    void showContextMenu();
    void setCheckedExclusive(QAction *action, const QString &name);

private:
    DTK_WIDGET_NAMESPACE::DToolButton *toolBtn { nullptr };
    QMenu *menu { nullptr };

    QString curEncodeName;
    bool isPressed { false };
};

#endif   // ENCODECOMBOBOX_H
