// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "instantblamewidget.h"
#include "client/gitclient.h"

#include "services/editor/editorservice.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QDateTime>
#include <QMouseEvent>

using namespace dpfservice;

InstantBlameWidget::InstantBlameWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void InstantBlameWidget::initUI()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    label = new QLabel(this);

    layout->addWidget(label);
}

CommitInfo InstantBlameWidget::parserBlameOutput(const QStringList &blame)
{
    CommitInfo result;
    if (blame.size() <= 12)
        return result;

    result.sha1 = blame.at(0).left(40);
    result.author = blame.at(1).mid(7);
    result.authorMail = blame.at(2).mid(13).chopped(1);
    const uint timeStamp = blame.at(3).mid(12).toUInt();
    result.authorTime = QDateTime::fromSecsSinceEpoch(timeStamp);
    result.summary = blame.at(9).mid(8);
    result.filePath = blame.at(10).mid(8);
    return result;
}

void InstantBlameWidget::setInfo(const QString &info)
{
    label->setToolTip(info);

    QString format(tr("Blame %1 (%2)"));
    auto ret = parserBlameOutput(info.split('\n'));
    label->setText(format.arg(ret.author, ret.authorTime.toString("yyyy-MM-dd")));
}

void InstantBlameWidget::clear()
{
    label->clear();
    label->setToolTip("");
}

void InstantBlameWidget::mousePressEvent(QMouseEvent *event)
{
    const auto &info = label->toolTip();
    if (info.isEmpty() || event->button() != Qt::LeftButton)
        return QWidget::mousePressEvent(event);

    auto editSrv = dpfGetService(EditorService);
    if (editSrv) {
        auto file = editSrv->currentFile();
        auto ret = parserBlameOutput(info.split('\n'));
        if (GitClient::instance()->show(file, ret.sha1))
            editSrv->switchWidget(GitWindow);
    }

    QWidget::mousePressEvent(event);
}
