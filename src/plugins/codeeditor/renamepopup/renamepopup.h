/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
