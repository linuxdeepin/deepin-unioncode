// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codelens.h"
#include "codelenstree.h"
#include "transceiver/codeeditorreceiver.h"

#include <DFrame>

#include <QGridLayout>

class CodeLensPrivate
{
    friend class CodeLens;
    CodeLensTree *lens {nullptr};
    QGridLayout *gLayout {nullptr};
    static CodeLens *ins;
};
CodeLens * CodeLensPrivate::ins {nullptr};

CodeLens *CodeLens::instance()
{
    if (!CodeLensPrivate::ins) {
        CodeLensPrivate::ins= new CodeLens;
    }
    return CodeLensPrivate::ins;
}

CodeLens::CodeLens(QWidget *parent)
    : QWidget(parent)
    , d (new CodeLensPrivate())
{
    d->lens = new CodeLensTree();
    d->gLayout = new QGridLayout();
    d->gLayout->addWidget(d->lens);
    d->gLayout->setMargin(0);
    
    QHBoxLayout *mainLayout = new QHBoxLayout();
    DFrame *mainFrame = new DFrame(this);
    mainLayout->addWidget(mainFrame);
    mainLayout->setMargin(0);
    mainFrame->setLayout(d->gLayout);
    mainFrame->setLineWidth(0);
    DStyle::setFrameRadius(mainFrame, 0);
    setLayout(mainLayout);
    
    connect(d->lens, &CodeLensTree::doubleClicked, EditorCallProxy::instance(), &EditorCallProxy::reqGotoPosition);
}

CodeLens::~CodeLens()
{
    if (d) {
        delete d;
    }
}

void CodeLens::displayReference(const lsp::References &data)
{
    uiController.switchContext(tr("Search &Results"));
    d->lens->setData(data);
}
