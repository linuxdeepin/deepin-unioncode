/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#ifndef LOADCOREDIALOG_H
#define LOADCOREDIALOG_H

#include <QDialog>

namespace ReverseDebugger {
namespace Internal {

class StartCoredumpDialogPrivate;
class CoredumpRunParameters
{
public:
    int pid = 0;

    // trace directory.
    QString tracedir;
};

class LoadCoreDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoadCoreDialog(QWidget *parent = nullptr);
    ~LoadCoreDialog();

    CoredumpRunParameters displayDlg(const QString &traceDir);

signals:

public slots:

private:
    void setupUi();
    void updatePid();
    void historyIndexChanged(int);
    void showFileDialog();

    StartCoredumpDialogPrivate *const d;
};

}   // namespace ReverseDebugger
}   // namespace Internal

#endif   // LOADCOREDIALOG_H
