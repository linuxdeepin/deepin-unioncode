// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODELENS_H
#define CODELENS_H

#include "codelenstype.h"

#include "common/common.h"

#include <QTreeView>

namespace dpfservice {
class EditorService;
}

class CodeLensTree : public QTreeView
{
    Q_OBJECT
public:
    explicit CodeLensTree(QWidget *parent = nullptr);
    void setData(const lsp::References &refs);

signals:
    void doubleClicked(const QString &filePath, int line, int column);

private:
    QString readLine(const QString &filePath, int line);
    QString readFileLine(const QString &filePath, int line);

    dpfservice::EditorService *editSrv { nullptr };
    bool onceReadFlag { false };
};
#endif   // CODELENS_H
