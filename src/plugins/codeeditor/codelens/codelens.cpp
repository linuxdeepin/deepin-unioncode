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
#include "codelens.h"
#include "codelenstree.h"
#include <QGridLayout>

class CodeLensPrivate
{
    friend class CodeLens;
    CodeLensTree *lens {nullptr};
    QGridLayout *gLayout {nullptr};
};

CodeLens *CodeLens::instance()
{
    static CodeLens ins;
    return &ins;
}

CodeLens::CodeLens(QWidget *parent)
    : QWidget(parent)
    , d (new CodeLensPrivate())
{
    d->lens = new CodeLensTree();
    d->gLayout = new QGridLayout();
    d->gLayout->addWidget(d->lens);
    setLayout(d->gLayout);
    QObject::connect(d->lens, &CodeLensTree::doubleClicked, this, &CodeLens::doubleClicked);
}

CodeLens::~CodeLens()
{
    if (d) {
        delete d;
    }
}

void CodeLens::displayReference(const lsp::References &data)
{
    d->lens->setData(data);
}
