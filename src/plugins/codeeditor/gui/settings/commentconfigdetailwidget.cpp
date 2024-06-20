// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commentconfigdetailwidget.h"
#include "settingsdefine.h"

#include <DLabel>
#include <DLineEdit>

#include <QFormLayout>

DWIDGET_USE_NAMESPACE

class CommentConfigDetailWidgetPrivate
{
    friend class CommentConfigDetailWidget;

    DLineEdit *tabNameEdit = nullptr;
    DLineEdit *mimeTypeEdit = nullptr;
    DLineEdit *singleLineCommentEdit = nullptr;
    DLineEdit *startMultiLineCommentEdit = nullptr;
    DLineEdit *endMultiLineCommentEdit = nullptr;
    
    int index = 0;
};

CommentConfigDetailWidget::CommentConfigDetailWidget(QWidget *parent, int index)
    : PageWidget(parent)
    , d(new CommentConfigDetailWidgetPrivate)
{
    initUI();
    initConnections();
    d->index = index;
}

CommentConfigDetailWidget::~CommentConfigDetailWidget()
{
    delete d;
}

void CommentConfigDetailWidget::initUI()
{
    QFormLayout *formLayout = new QFormLayout(this);
    formLayout->setContentsMargins(30, 20, 50, 20);
    formLayout->setSpacing(10);
    
    DLabel *tabName = new DLabel(tr("Group Name:"), this);
    d->tabNameEdit = new DLineEdit(this);
    d->tabNameEdit->setText("New Group");

    DLabel *mimeType = new DLabel(tr("MIME Type:"), this);
    d->mimeTypeEdit = new DLineEdit(this);
    d->mimeTypeEdit->setText("text/x-c++src;text/x-c++hdr;");

    DLabel *singleLineComment = new DLabel(tr("Single-line comment symbol:"), this);
    d->singleLineCommentEdit = new DLineEdit(this);
    d->singleLineCommentEdit->setText("//");

    DLabel *startMultiLineComment = new DLabel(tr("Single-line comment start symbol:"), this);
    d->startMultiLineCommentEdit = new DLineEdit(this);
    d->startMultiLineCommentEdit->setText("/*");

    DLabel *endMultiLineComment = new DLabel(tr("Single-line comment end symbol:"), this);
    d->endMultiLineCommentEdit = new DLineEdit(this);
    d->endMultiLineCommentEdit->setText("*/");

    formLayout->addRow(tabName, d->tabNameEdit);
    formLayout->addRow(mimeType, d->mimeTypeEdit);
    formLayout->addRow(singleLineComment, d->singleLineCommentEdit);
    formLayout->addRow(startMultiLineComment, d->startMultiLineCommentEdit);
    formLayout->addRow(endMultiLineComment, d->endMultiLineCommentEdit);
}

void CommentConfigDetailWidget::initConnections()
{
    connect(d->tabNameEdit, &DLineEdit::editingFinished, this, [=]() {
        if (d->tabNameEdit->text().isEmpty()) {
            d->tabNameEdit->showAlertMessage(tr("The group name cannot be empty"), 1000);
            d->tabNameEdit->setText("Group Name");
        }
    });
}

void CommentConfigDetailWidget::getUserConfig(QMap<QString, QVariant> &map)
{
    Q_UNUSED(map)
    
    QVariantMap qVariantMap;
    qVariantMap.insert(Key::MimeTypeGroupName, d->tabNameEdit->text());
    qVariantMap.insert(Key::MimeType, d->mimeTypeEdit->text());
    qVariantMap.insert(Key::SingleLineComment, d->singleLineCommentEdit->text());
    qVariantMap.insert(Key::StartMultiLineComment, d->startMultiLineCommentEdit->text());
    qVariantMap.insert(Key::EndMultiLineComment, d->endMultiLineCommentEdit->text());

    map.insert(QString::number(d->index), qVariantMap);
}

void CommentConfigDetailWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    QVariantMap qVariantMap = map.value(QString::number(d->index)).toMap();
    QVariant tabName = qVariantMap.value(Key::MimeTypeGroupName);
    QVariant mimeType = qVariantMap.value(Key::MimeType);
    QVariant singleLineComment = qVariantMap.value(Key::SingleLineComment);
    QVariant startMultiLineComment = qVariantMap.value(Key::StartMultiLineComment);
    QVariant endMultiLineComment = qVariantMap.value(Key::EndMultiLineComment);
    
    d->tabNameEdit->setText(tabName.toString());
    d->mimeTypeEdit->setText(mimeType.toString());
    d->singleLineCommentEdit->setText(singleLineComment.toString());
    d->startMultiLineCommentEdit->setText(startMultiLineComment.toString());
    d->endMultiLineCommentEdit->setText(endMultiLineComment.toString());
}


