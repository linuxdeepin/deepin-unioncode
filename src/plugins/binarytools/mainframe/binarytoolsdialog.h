/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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

#ifndef BINARYTOOLSDIALOG_H
#define BINARYTOOLSDIALOG_H

#include "common/widget/outputpane.h"

#include <QDialog>

class BinaryToolsDialogPrivate;
class BinaryToolsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BinaryToolsDialog(QDialog *parent = nullptr);
    ~BinaryToolsDialog();

signals:
    void sigOutputMsg(const QString &content, OutputPane::OutputFormat format = OutputPane::OutputFormat::NormalMessage);

public slots:
    void printOutput(const QString &content, OutputPane::OutputFormat format);
    void saveClicked();
    void useClicked();

private:
    void outputMsg(const QString &content, OutputPane::OutputFormat format);

    BinaryToolsDialogPrivate *const d = nullptr;
};

#endif // BINARYTOOLSDIALOG_H
