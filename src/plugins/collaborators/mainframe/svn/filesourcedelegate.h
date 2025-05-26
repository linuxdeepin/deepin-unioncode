// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESOURCEDELEGATE_H
#define FILESOURCEDELEGATE_H

#include <QStyledItemDelegate>

class FileSourceDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FileSourceDelegate(QObject *parent = nullptr);
};

#endif // FILESOURCEDELEGATE_H
