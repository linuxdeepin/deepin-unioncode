// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commentconfigdetailwidget.h"
#include "settingsdefine.h"
#include "editorsettings.h"

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

    DLabel *singleLineComment = new DLabel(tr("Line comment symbol:"), this);
    d->singleLineCommentEdit = new DLineEdit(this);
    d->singleLineCommentEdit->setText("//");

    DLabel *startMultiLineComment = new DLabel(tr("Block comment start symbol:"), this);
    d->startMultiLineCommentEdit = new DLineEdit(this);
    d->startMultiLineCommentEdit->setText("/*");

    DLabel *endMultiLineComment = new DLabel(tr("Block comment end symbol:"), this);
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
    
    EditorSettings::instance()->setValue(Node::MimeTypeConfig, QString::number(d->index), Key::MimeTypeGroupName, d->tabNameEdit->text());
    EditorSettings::instance()->setValue(Node::MimeTypeConfig, QString::number(d->index), Key::MimeType, d->mimeTypeEdit->text());
    EditorSettings::instance()->setValue(Node::MimeTypeConfig, QString::number(d->index), Key::SingleLineComment, d->singleLineCommentEdit->text());
    EditorSettings::instance()->setValue(Node::MimeTypeConfig, QString::number(d->index), Key::StartMultiLineComment, d->startMultiLineCommentEdit->text());
    EditorSettings::instance()->setValue(Node::MimeTypeConfig, QString::number(d->index), Key::EndMultiLineComment, d->endMultiLineCommentEdit->text());
}

void CommentConfigDetailWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    QVariantMap qVariantMap = EditorSettings::instance()->getMap(Node::MimeTypeConfig).value(QString::number(d->index)).toMap();
    auto tabName = qVariantMap.value(Key::MimeTypeGroupName);
    auto mimeType = qVariantMap.value(Key::MimeType);
    auto singleLineComment = qVariantMap.value(Key::SingleLineComment);
    auto startMultiLineComment = qVariantMap.value(Key::StartMultiLineComment);
    auto endMultiLineComment = qVariantMap.value(Key::EndMultiLineComment);

    d->tabNameEdit->setText(tabName.toString());
    d->mimeTypeEdit->setText(mimeType.toString());
    d->singleLineCommentEdit->setText(singleLineComment.toString());
    d->startMultiLineCommentEdit->setText(startMultiLineComment.toString());
    d->endMultiLineCommentEdit->setText(endMultiLineComment.toString());
}


