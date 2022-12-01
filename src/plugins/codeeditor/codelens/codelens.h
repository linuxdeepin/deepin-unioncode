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
#ifndef REFACTORWIDGET_H
#define REFACTORWIDGET_H

#include "common/common.h"
#include <QWidget>

class CodeLensPrivate;
class CodeLens : public QWidget
{
    Q_OBJECT
    CodeLensPrivate * const d;
public:
    static CodeLens *instance();
    explicit CodeLens(QWidget *parent = nullptr);
    virtual ~CodeLens();
    void displayReference(const lsp::References &data);

signals:
    void doubleClicked(const QString &filePath, const lsp::Range range);
};

#endif // REFACTORWIDGET_H
