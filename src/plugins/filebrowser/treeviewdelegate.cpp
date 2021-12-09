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
