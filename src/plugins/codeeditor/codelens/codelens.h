// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
