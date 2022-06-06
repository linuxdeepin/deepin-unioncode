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
#include "amendswidget.h"
#include "CommitInfo.h"
#include "filesourceview.h"
#include "filemodifyview.h"
#include "CommitInfoPanel.h"

#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

const QString Description = QTextEdit::tr("Description");
const QString Summary = QLineEdit::tr("Summary");
const QString Commit = QPushButton::tr("Commit");
const QString RevertAll = QPushButton::tr("Revert All");

AmendsWidget::AmendsWidget(QWidget *parent)
    : QSplitter (parent)
    , modifyView(new FileModifyView)
    , hLayPbt(new QHBoxLayout)
    , pbtCommit(new QPushButton(Commit))
    , pbtRevertAll(new QPushButton(RevertAll))
    , descEdit(new QTextEdit)
    , pbtWidget(new QFrame)
{
    setOrientation(Qt::Vertical);
    pbtRevertAll->setObjectName("warningButton");
    pbtCommit->setObjectName("applyActionBtn");
    QObject::connect(pbtRevertAll, &QPushButton::clicked,
                     this, &AmendsWidget::revertAllClicked);
    QObject::connect(pbtCommit, &QPushButton::clicked,
                     this, &AmendsWidget::commitClicked);
    descEdit->setPlaceholderText(Description);
    descEdit->setObjectName("teDescription");
    addWidget(descEdit);
    addWidget(modifyView);
    setHandleWidth(2);
    hLayPbt->addWidget(pbtRevertAll);
    hLayPbt->addWidget(pbtCommit);
    pbtWidget->setLayout(hLayPbt);
    pbtWidget->setFixedHeight(45);
    pbtWidget->setObjectName("teDescription");
    addWidget(pbtWidget);
}

AmendsWidget::~AmendsWidget()
{
}

QString AmendsWidget::description()
{
    return descEdit->toPlainText();
}

FileModifyView *AmendsWidget::modView()
{
    return modifyView;
}
