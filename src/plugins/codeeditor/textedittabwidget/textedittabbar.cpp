// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textedittabbar.h"
#include "common/common.h"
#include "services/editor/editorservice.h"

#include <DToolButton>
#include <DMenu>

#include <QClipboard>
#include <QFileInfo>
#include <QDebug>
#include <QMessageBox>
#include <QVBoxLayout>

class TextEditTabBarPrivate
{
    friend class TextEditTabBar;
    QTabBar *tab = nullptr;
    QHBoxLayout * hBoxLayout = nullptr;
    DToolButton *pbtHorizontal = nullptr;
    DToolButton *pbtVertical = nullptr;
    DToolButton *pbtClose = nullptr;
};

TextEditTabBar::TextEditTabBar(QWidget *parent)
    : DFrame (parent)
    , d(new TextEditTabBarPrivate)
{
    d->tab = new QTabBar(this);
    d->hBoxLayout = new QHBoxLayout();
    d->pbtHorizontal = new DToolButton();
    d->pbtVertical = new DToolButton();
    d->pbtClose = new DToolButton(this);
    d->hBoxLayout->setSpacing(5);
    d->hBoxLayout->setMargin(0);

    d->pbtHorizontal->setIcon(QIcon::fromTheme("edit-hSplit"));
    d->pbtVertical->setIcon(QIcon::fromTheme("edit-vSplit"));
    d->pbtClose->setIcon(QIcon::fromTheme("edit-closeBtn"));
    d->hBoxLayout->addWidget(d->tab);
    d->hBoxLayout->addStretch(10);
    d->hBoxLayout->addWidget(d->pbtHorizontal);
    d->hBoxLayout->addWidget(d->pbtVertical);
    d->hBoxLayout->addWidget(d->pbtClose);
    d->tab->setTabsClosable(true);

    d->tab->setContextMenuPolicy(Qt::CustomContextMenu);

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

    QObject::connect(d->pbtHorizontal, &DToolButton::clicked,
                     this, [=]() {
        emit splitClicked(Qt::Vertical);
    });

    QObject::connect(d->pbtVertical, &DToolButton::clicked,
                     this, [=]() {
        emit splitClicked(Qt::Horizontal);
    });    

    QObject::connect(d->pbtClose, &DToolButton::clicked,
                     this, [=]() {
        emit closeClicked();
    });

    QObject::connect(d->tab, &DTabBar::customContextMenuRequested,
                     this, &TextEditTabBar::showMenu);

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
    d->tab->setTabData(addIndex, info.fileName());
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

void TextEditTabBar::showMenu(QPoint pos)
{
    int curIndex = d->tab->tabAt(pos);
    DMenu *menu = new DMenu(this);

    menu->addAction(tr("Copy File Path"), [=]() {
        auto file = d->tab->tabToolTip(curIndex);
        qApp->clipboard()->setText(file);
    });
    menu->addAction(tr("Copy File Name"), [=]() {
        auto fileName = d->tab->tabData(curIndex).toString();
        qApp->clipboard()->setText(fileName);
    });

    menu->addSeparator();

    menu->addAction(tr("Close This File"), [=]() {
        auto file = d->tab->tabToolTip(curIndex);
        removeTab(file);
    });
    menu->addAction(tr("Close All Files"), [=]() {
        while(d->tab->count() > 0) {
            auto file = d->tab->tabToolTip(0);
            removeTab(file);
        };
    });
    menu->addAction(tr("Close All Files Except This"), [=]() {
        auto curFile = d->tab->tabToolTip(curIndex);
        int index = 0;
        while(d->tab->count() > 1) {
            auto file = d->tab->tabToolTip(index);
            if (file != curFile)
                removeTab(file);
            else
                index++;
        };
    });

    menu->addSeparator();

    menu->addAction(tr("Open File Location"), [=]() {
        auto file = d->tab->tabToolTip(curIndex);
        QProcess::startDetached("dde-file-manager", QStringList() << "--show-item" << file << "--raw");
    });

    menu->exec(QCursor::pos());
    menu->deleteLater();
}
