// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTFILTERDIALOG_H
#define EVENTFILTERDIALOG_H

#include <DDialog>

class QHBoxLayout;
class QTreeWidgetItem;

namespace ReverseDebugger {
namespace Internal {

class EventFilterDialogPrivate;
class EventFilterDialog : public DTK_WIDGET_NAMESPACE::DDialog
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
    void setupButton(QHBoxLayout *buttonLayout);

    EventFilterDialogPrivate *const d;
};

}   // namespace Debugger
}   // namespace Internal

#endif   // EVENTFILTERDIALOG_H
