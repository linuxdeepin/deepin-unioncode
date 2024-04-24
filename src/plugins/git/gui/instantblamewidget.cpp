// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "instantblamewidget.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QDateTime>

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
    return result;
}

void InstantBlameWidget::setInfo(const QString &info)
{
    label->setToolTip(info);

    QString format(tr("Blame %1 (%2)"));
    auto ret = parserBlameOutput(info.split('\n'));
    label->setText(format.arg(ret.author, ret.authorTime.toString("yyyy-MM-dd")));
}
