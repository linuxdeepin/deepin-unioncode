// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODELENS_H
#define CODELENS_H

#include "codelenstype.h"

#include "common/common.h"

#include <QTreeView>

class CodeLensTree : public QTreeView
{
    Q_OBJECT
public:
    explicit CodeLensTree(QWidget *parent = nullptr);
    void setData(const lsp::References &refs);

signals:
    void doubleClicked(const QString &filePath, const lsp::Range &range);
};
#endif // CODELENS_H
