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
#include "renamepopup.h"

#include "common/common.h"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QEventLoop>

class RenamePopupPrivate
{
    friend class RenamePopup;
    QString oldName{""};
    QLineEdit *renameEdit{nullptr};
    QLabel *renameLabel{nullptr};
    QVBoxLayout *vLayout{nullptr};
    QEventLoop *loop{nullptr};
};

RenamePopup *RenamePopup::instance()
{
    static RenamePopup ins;
    return &ins;
}

RenamePopup::RenamePopup(QWidget *parent)
    : d(new RenamePopupPrivate())
{

    RenamePopup::setWindowFlag(Qt::Popup, true);

    d->loop = new QEventLoop();
    d->renameEdit = new QLineEdit();
    d->renameLabel = new QLabel();
    d->vLayout = new QVBoxLayout();

    QObject::connect(d->renameEdit, &QLineEdit::returnPressed, [=](){
        emit this->editingFinished(d->renameEdit->text());
        this->close();
        d->loop->quit();
    });

    d->vLayout->addWidget(d->renameLabel);
    d->vLayout->addWidget(d->renameEdit);
    RenamePopup::setLayout(d->vLayout);
}

RenamePopup::~RenamePopup()
{
    if (d) {
        if (d->loop) {
            d->loop->quit();
            delete d->loop;
        }
        delete d;
    }
}

void RenamePopup::setOldName(const QString &name)
{
    d->oldName = name;
}

QString RenamePopup::oldName()
{
    return d->oldName;
}

int RenamePopup::exec(const QPoint &pos)
{
    QEventLoop loop;
    this->move(pos);
    this->show();
    return loop.exec();
}

int RenamePopup::exec()
{
    QEventLoop loop;
    this->show();
    return loop.exec();
}

void RenamePopup::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    d->renameLabel->setText(RenamePopup::tr("Rename %0 to:").arg(d->oldName));
    d->renameEdit->setFocus();
}

void RenamePopup::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    QWidget *parent = qobject_cast<QWidget*>(this->parent());
    if (parent) {
        parent->setFocus();
    }
}
