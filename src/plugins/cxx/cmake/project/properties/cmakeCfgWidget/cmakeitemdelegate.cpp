// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmakeitemdelegate.h"
#include "cmakeitemmodel.h"

#include <DFileDialog>

#include <QDebug>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

pathChooser::pathChooser(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->setContentsMargins(0, 0, 0, 0);

    edit = new DLineEdit(parent);
    browser = new DSuggestButton("...", parent);

    setFocusProxy(edit);
    setFocusPolicy(edit->focusPolicy());

    connect(browser, &DSuggestButton::clicked, this, [=]() {
        QString path = DFileDialog::getExistingDirectory(parent, tr("Choose path"), QDir::homePath());
        if (!path.isEmpty())
            setPath(path);
    });

    hlayout->addWidget(edit);
    hlayout->addWidget(browser, 0, Qt::AlignRight);
}

QString pathChooser::getPath()
{
    return edit->text();
}

void pathChooser::setPath(const QString path)
{
    edit->setText(path);
}

CMakeItemDelegate::CMakeItemDelegate(QObject *parent)
    : BaseItemDelegate(parent)
{
}

QWidget *CMakeItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 1) {
        auto model = static_cast<const CMakeItemModel *>(index.model());
        auto item = model->itemOfIndex(index);
        if (item.type == CMakeItem::itemType::FILEPATH || item.type == CMakeItem::itemType::PATH) {
            return new pathChooser(parent);
        } else if (item.type == CMakeItem::itemType::STRING) {
            return new DLineEdit(parent);
        }
    }

    return QStyledItemDelegate::createEditor(parent, option, index);
}

void CMakeItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 1) {
        auto model = static_cast<const CMakeItemModel *>(index.model());
        auto item = model->itemOfIndex(index);

        if (item.type == CMakeItem::itemType::FILEPATH || item.type == CMakeItem::itemType::PATH) {
            auto pathChosser = static_cast<pathChooser *>(editor);
            pathChosser->setPath(item.value.toString());
            return ;
        } else if (item.type == CMakeItem::itemType::STRING) {
            auto lineEdit = static_cast<DLineEdit *>(editor);
            lineEdit->setText(item.value.toString());
            return ;
        }
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

void CMakeItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column() == 1) {
        auto cmakeModel = static_cast<CMakeItemModel *>(model);
        auto item = cmakeModel->itemOfIndex(index);

        if (item.type == CMakeItem::itemType::FILEPATH || item.type == CMakeItem::itemType::PATH) {
            auto pathChosser = static_cast<pathChooser *>(editor);
            auto path = pathChosser->getPath();
            cmakeModel->setData(index, path, Qt::EditRole);
            return ;
        } else if (item.type == CMakeItem::itemType::STRING) {
            auto lineEdit = static_cast<DLineEdit *>(editor);
            cmakeModel->setData(index, lineEdit->text(), Qt::EditRole);
            return ;
        }
    }

    QStyledItemDelegate::setModelData(editor, model, index);
}
