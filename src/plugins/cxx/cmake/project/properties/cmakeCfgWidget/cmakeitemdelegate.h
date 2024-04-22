// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMAKEITEMDELEGATE_H
#define CMAKEITEMDELEGATE_H

#include "base/baseitemdelegate.h"

#include <DLineEdit>
#include <DSuggestButton>

#include <QStyledItemDelegate>

class pathChooser : public QWidget
{
public:
    pathChooser(QWidget *parent = nullptr);

    QString getPath();
    void setPath(const QString path);

private:
    DTK_WIDGET_NAMESPACE::DLineEdit *edit { nullptr };
    DTK_WIDGET_NAMESPACE::DSuggestButton *browser { nullptr };
};

class CMakeItemDelegate : public BaseItemDelegate
{
public:
    CMakeItemDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const final;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const final;
};

#endif // CMAKEITEMDELEGATE_H
