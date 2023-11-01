// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textedittabbar.h"
#include "common/common.h"
#include "services/editor/editorservice.h"

#include <QFileInfo>
#include <QDebug>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QToolButton>

class TextEditTabBarPrivate
{
    friend class TextEditTabBar;
    QTabBar *tab = nullptr;
    QHBoxLayout * hBoxLayout = nullptr;
    QToolButton *pbtHorizontal = nullptr;
    QToolButton *pbtVertical = nullptr;
    QToolButton *pbtClose = nullptr;
};

TextEditTabBar::TextEditTabBar(QWidget *parent)
    : QWidget (parent)
    , d(new TextEditTabBarPrivate)
{
    d->tab = new QTabBar(this);
    d->hBoxLayout = new QHBoxLayout();
    d->pbtHorizontal = new QToolButton();
    d->pbtVertical = new QToolButton();
    d->pbtClose = new QToolButton(this);
    d->hBoxLayout->setSpacing(5);
    d->hBoxLayout->setMargin(0);

    d->pbtHorizontal->setIcon(QIcon(":/core/images/splitbutton_horizontal.png"));
    d->pbtVertical->setIcon(QIcon(":/core/images/splitbutton_vertical.png"));
    d->pbtClose->setIcon(QIcon(":/core/images/close_button_selected.png"));
    d->hBoxLayout->addWidget(d->tab);
    d->hBoxLayout->addStretch(10);
    d->hBoxLayout->addWidget(d->pbtHorizontal);
    d->hBoxLayout->addWidget(d->pbtVertical);
    d->hBoxLayout->addWidget(d->pbtClose);
    d->tab->setTabsClosable(true);

    this->setLayout(d->hBoxLayout);

    QObject::connect(d->tab, &QTabBar::currentChanged,
                     this, [=](int index){
        QString filePath = indexFile(index);
        emit this->fileSwitched(filePath);
        editor.switchedFile(filePath);
    });

    QObject::connect(d->tab, &QTabBar::tabCloseRequested,
                     this, [=](int index) {
        this->removeTab(this->indexFile(index));
    });

    QObject::connect(d->pbtHorizontal, &QToolButton::clicked,
                     this, [=]() {
        emit splitClicked(Qt::Horizontal);
    });

    QObject::connect(d->pbtVertical, &QToolButton::clicked,
                     this, [=]() {
        emit splitClicked(Qt::Vertical);
    });

    QObject::connect(d->pbtClose, &QToolButton::clicked,
                     this, [=]() {
        emit closeClicked();
    });
}

TextEditTabBar::~TextEditTabBar()
{
    if (d) {
        if (d->tab)
            delete d->tab;
        if (d->pbtClose)
            delete d->pbtClose;
        if (d->pbtHorizontal)
            delete d->pbtHorizontal;
        if (d->pbtVertical)
            delete d->pbtVertical;
        if (d->hBoxLayout)
            delete d->hBoxLayout;
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

    // should emit index changed, to use addTab method with tab is empty
    QFileInfo info(file);
    int addIndex = d->tab->addTab(info.fileName());
    d->tab->setTabToolTip(addIndex, file);

    editor.openedFile(file); // plugin interface
}

void TextEditTabBar::switchFile(const QString &file)
{
    int index = fileIndex(file);
    if (index != -1) {
        d->tab->setCurrentIndex(index);
    }
}

int TextEditTabBar::fileIndex(const QString &file) const
{
    int index = -1;
    for (int i = 0; i < d->tab->count(); i++) {
        if (d->tab->tabToolTip(i) == file)
            index = i;
    }
    return index;
}

QString TextEditTabBar::indexFile(int index) const
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
        emit fileClosed(file);
        editor.closedFile(file);
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

QString TextEditTabBar::currentFile() const
{
    return indexFile(currentIndex());
}

void TextEditTabBar::setCurrentIndex(int idx)
{
    return d->tab->setCurrentIndex(idx);
}

void TextEditTabBar::setCloseButtonVisible(bool flag)
{
    d->pbtClose->setVisible(flag);
}

void TextEditTabBar::setSplitButtonVisible(bool flag)
{
    d->pbtHorizontal->setVisible(flag);
    d->pbtVertical->setVisible(flag);
}

void TextEditTabBar::tabCloseRequested(int idx)
{
    return d->tab->tabCloseRequested(idx);
}
