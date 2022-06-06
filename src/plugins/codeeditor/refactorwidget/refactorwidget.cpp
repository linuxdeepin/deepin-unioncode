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
#include "refactorwidget.h"
#include "codelens.h"
#include <QGridLayout>

class RefactorWidgetPrivate
{
    friend class RefactorWidget;
    CodeLens *lens {nullptr};
    QGridLayout *gLayout {nullptr};
};

RefactorWidget *RefactorWidget::instance()
{
    static RefactorWidget *ins = new RefactorWidget;
    return ins;
}

RefactorWidget::RefactorWidget(QWidget *parent)
    : QWidget(parent)
    , d (new RefactorWidgetPrivate())
{
    d->lens = new CodeLens();
    d->gLayout = new QGridLayout();
    d->gLayout->addWidget(d->lens);
    setLayout(d->gLayout);
    QObject::connect(d->lens, &CodeLens::doubleClicked, this, &RefactorWidget::doubleClicked);
}

RefactorWidget::~RefactorWidget()
{
    if (d) {
        delete d;
    }
}

void RefactorWidget::displayReference(const lsp::References &data)
{
    d->lens->setData(data);
}
