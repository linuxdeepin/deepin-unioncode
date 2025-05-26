// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "amendswidget.h"
#include "CommitInfo.h"
#include "filesourceview.h"
#include "filemodifyview.h"
#include "CommitInfoPanel.h"

#include <QBoxLayout>

const QString Description = QTextEdit::tr("Description");
const QString Summary = QLineEdit::tr("Summary");
const QString Commit = QPushButton::tr("Commit");
const QString RevertAll = QPushButton::tr("Revert All");

AmendsWidget::AmendsWidget(QWidget *parent)
    : DSplitter (parent)
    , modifyView(new FileModifyView)
    , hLayPbt(new QHBoxLayout)
    , pbtCommit(new DPushButton(Commit))
    , pbtRevertAll(new DPushButton(RevertAll))
    , descEdit(new DTextEdit)
    , pbtWidget(new DFrame)
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
