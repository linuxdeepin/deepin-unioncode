/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "treeviewdelegate.h"

#include <QLineEdit>
#include <QDebug>
#include <QToolTip>

const QRegExp regSep {"[^\\\\ /?@#$&|<>]+"};

TreeViewDelegate::TreeViewDelegate(QObject *parent)
    : QItemDelegate (parent)
{

}

QWidget* TreeViewDelegate::createEditor(QWidget* parent,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const
{
    QWidget *editor = QItemDelegate::createEditor(parent, option, index);
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) {

        QObject::connect(lineEdit, &QLineEdit::textChanged, lineEdit, [=](const QString &text){
            QString failed = " file name invalid";
            QString tipText;
            if (text == "..") tipText = QString("Rename ") + "\'..\'" + failed;
            if (text == ".") tipText = QString("Rename ") + "\'.\'" + failed;
            auto showPos = lineEdit->pos();
            showPos = lineEdit->window()->mapToGlobal(QPoint{lineEdit->pos().x(),
                                                             lineEdit->pos().y()});
            QToolTip::showText(showPos, tipText, lineEdit);
        });

        QRegExpValidator *validator = new QRegExpValidator(regSep, lineEdit);
        lineEdit->setText(index.data(Qt::DisplayRole).toString());
        lineEdit->setValidator(validator);
        return lineEdit;
    }

    return editor;
}

void TreeViewDelegate::setEditorData(QWidget* editor,const QModelIndex& index) const
{
    if(index.isValid()) {
        QLineEdit* lineE = qobject_cast<QLineEdit*>(editor);
        lineE->setText(index.data(Qt::DisplayRole).toString());
    }
}

void TreeViewDelegate::setModelData(QWidget* editor,
                                    QAbstractItemModel* model,
                                    const QModelIndex& index
                                    ) const
{
    if(index.isValid()) {
        QLineEdit* lineE = qobject_cast<QLineEdit*>(editor);
        model->setData(index,lineE->text());
    }
}
