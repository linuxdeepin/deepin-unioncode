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
#ifndef EVENTFILTERDIALOG_H
#define EVENTFILTERDIALOG_H

#include <QDialog>

class QTreeWidgetItem;

namespace ReverseDebugger {
namespace Internal {

class EventFilterDialogPrivate;
class EventFilterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EventFilterDialog(QWidget *parent,
                               uchar *syscallFlags,
                               uchar *dbusFlags,
                               uchar *x11Flags,
                               uchar *signalFlags);
    ~EventFilterDialog();

    int exec();

    QString syscallKindNames(void) const;
    QString breakFunc(void) const;
    QString globalVar(void) const;

    QString maxStackSize(void) const;
    QString maxHeapSize(void) const;
    QString maxParamSize(void) const;
    bool onlyCurrentThread(void) const;

    void setMaxStackSize(const QString &size);
    void setMaxHeapSize(const QString &size);
    void setMaxParamSize(const QString &size);
    void setOnlyCurrentThread(bool b);

signals:

private slots:
    void itemClicked(QTreeWidgetItem *item, int column);

private:
    void setupUi();

    EventFilterDialogPrivate *const d;
};

}   // namespace Debugger
}   // namespace Internal

#endif   // EVENTFILTERDIALOG_H
