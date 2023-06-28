// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RENAMEPOPUP_H
#define RENAMEPOPUP_H

#include "common/common.h"

#include <QWidget>

class RenamePopupPrivate;
class RenamePopup : public QWidget
{
    Q_OBJECT
    RenamePopupPrivate *const d;
public:
    static RenamePopup *instance();
    explicit RenamePopup(QWidget *parent = nullptr);
    virtual ~RenamePopup();
    void setOldName(const QString &name);
    QString oldName();
    virtual int exec(const QPoint &pos);
    virtual int exec();

signals:
    void editingFinished(const QString &text);

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);
};

#endif // RENAMEPOPUP_H
