// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESOURCEVIEW_H
#define FILESOURCEVIEW_H

#include "basetype.h"

#include <DTreeView>

class QVBoxLayout;
class FileSourceViewPrivate;
class FileSourceView : public DTreeView
{
    Q_OBJECT
    FileSourceViewPrivate *const d;
public:
    explicit FileSourceView(QWidget *parent = nullptr);
    void setRootPath(const QString &filePath);
};

#endif // FILESOURCEVIEW_H
