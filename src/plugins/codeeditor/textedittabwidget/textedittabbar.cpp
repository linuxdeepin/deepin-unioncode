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
#include "textedittabbar.h"
#include "transceiver/sendevents.h"
#include <QFileInfo>
#include <QDebug>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QToolButton>

class TextEditTabBarPrivate
{
    friend class TextEditTabBar;
    QTabBar *tab = new QTabBar();
    QHBoxLayout * hBoxLayout = new QHBoxLayout();
    QToolButton *pbtHorizontal = new QToolButton;
//    QPushButton *pbtVertical = new QPushButton("vertical");
    QToolButton *pbtClose = new QToolButton;
};

TextEditTabBar::TextEditTabBar(QWidget *parent)
    : QWidget (parent)
    , d(new TextEditTabBarPrivate)
{
    d->hBoxLayout->setSpacing(5);
    d->hBoxLayout->setMargin(0);
    d->tab->setFixedHeight(24);

    d->pbtHorizontal->setIcon(QIcon(":/core/images/horizontalSplit.png"));
    d->pbtHorizontal->setStyleSheet("background-color:transparent");
//    d->pbtVertical->setMaximumSize(40,40);
    d->pbtClose->setMaximumSize(24,24);
    d->pbtClose->setIcon(QIcon(":/core/images/closeButton.png"));
    d->pbtClose->setStyleSheet("background-color:transparent");
    d->hBoxLayout->addWidget(d->tab, 0, Qt::AlignLeft | Qt::AlignTop);
    d->hBoxLayout->addWidget(d->pbtHorizontal, 0, Qt::AlignRight | Qt::AlignTop);
    d->hBoxLayout->addWidget(d->pbtClose);
//    d->hBoxLayout->addWidget(d->pbtVertical);
    d->tab->setTabsClosable(true);
    d->tab->setExpanding(false);

    this->setLayout(d->hBoxLayout);

    QObject::connect(d->tab, &QTabBar::currentChanged, this, [=](int index){
        emit this->fileSwitched(indexFile(index));
    });

    QObject::connect(d->tab, &QTabBar::tabCloseRequested,
                     this, [=](int index) {
        this->removeTab(this->indexFile(index));
    });

    QObject::connect(d->pbtHorizontal, &QToolButton::clicked,
                     this, [=]() {
        int index = d->tab->currentIndex();
        QString file = indexFile(index);
        emit horizontalSplit(file);
    });

//    QObject::connect(d->pbtVertical, &QPushButton::clicked,
//                     this, [=]() {
//        emit verticalSplit();
//    });

    QObject::connect(d->pbtClose, &QToolButton::clicked,
                     this, [=]() {
        emit close();
    });

}

TextEditTabBar::~TextEditTabBar()
{
    if (d) {
        delete d;
    }
}

void TextEditTabBar::setFile(const QString &file)
{
    if (!QFile::exists(file)) {
        return;
    }

    int index = fileIndex(file);
    if (index != -1) {
        d->tab->setCurrentIndex(index);
        return;
    }

    QFileInfo info(file);
    int addIndex = d->tab->addTab(info.fileName());
    d->tab->setTabToolTip(addIndex, file);
}

void TextEditTabBar::switchFile(const QString &file)
{
    int index = fileIndex(file);
    if (index != -1) {
        d->tab->setCurrentIndex(index);
        SendEvents::sendCurrentEditFileStatus(file, true);
    }
}

int TextEditTabBar::fileIndex(const QString &file)
{
    int index = -1;
    for (int i = 0; i < d->tab->count(); i++) {
        if (d->tab->tabToolTip(i) == file)
            index = i;
    }
    return index;
}

QString TextEditTabBar::indexFile(int index)
{
    return d->tab->tabToolTip(index);
}

void TextEditTabBar::doFileChanged(const QString &file)
{
    int index = fileIndex(file);
    if (index == -1)
        return;

    QString changedFileName = "*" + QFileInfo(file).fileName();
    if (d->tab->tabText(index) == changedFileName) {
        return;
    }

    d->tab->setTabText(index , "*" + d->tab->tabText(index));
    qInfo() << d->tab->tabText(index);
}

void TextEditTabBar::doFileSaved(const QString &file)
{
    int index = fileIndex(file);
    if (index == -1)
        return;

    QString text = d->tab->tabText(index);
    if (QFileInfo(file).fileName() == text){
        return;
    }

    text = text.remove(0, 1);
    d->tab->setTabText(index, text);
}

void TextEditTabBar::removeTab(const QString &file)
{
    int index = fileIndex(file);
    if (index != -1){
        QString text = d->tab->tabText(index);
        QFileInfo info(file);
        if (info.exists() && text.length() > 0 && text.at(0) == "*") {
            int ret = QMessageBox::question(this, QMessageBox::tr("Save Changes"),
                                            QMessageBox::tr("The file has unsaved changes, will save?"),
                                            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                            QMessageBox::Cancel);
            if (QMessageBox::Yes != ret && QMessageBox::No != ret) {
                return;
            } else if (QMessageBox::Yes == ret) {
                emit saveFile(file);
            }
        }
        emit fileClosed(indexFile(index));
        SendEvents::sendCurrentEditFileStatus(file, false);
        d->tab->removeTab(index);
    }
}

int TextEditTabBar::count() const
{
    return d->tab->count();
}

int TextEditTabBar::currentIndex() const
{
    return d->tab->currentIndex();
}

void TextEditTabBar::setCurrentIndex(int idx)
{
    return d->tab->setCurrentIndex(idx);
}

void TextEditTabBar::tabCloseRequested(int idx)
{
    return d->tab->tabCloseRequested(idx);
}
