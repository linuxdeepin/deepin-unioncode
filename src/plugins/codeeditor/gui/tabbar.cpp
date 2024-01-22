// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabbar.h"
#include "private/tabbar_p.h"

#include "common/common.h"
#include "services/editor/editorservice.h"

#include <QFileInfo>
#include <QHBoxLayout>

TabBarPrivate::TabBarPrivate(TabBar *qq)
    : QObject(qq),
      q(qq)
{
}

void TabBarPrivate::initUI()
{
    tabBar = new DTabBar(q);
    tabBar->setVisibleAddButton(false);
    tabBar->setTabsClosable(true);
    tabBar->setEnabledEmbedStyle(true);
    hSplitBtn = new DToolButton(q);
    hSplitBtn->setIcon(QIcon::fromTheme("edit-hSplit"));
    vSplitBtn = new DToolButton(q);
    vSplitBtn->setIcon(QIcon::fromTheme("edit-vSplit"));
    closeBtn = new DToolButton(q);
    closeBtn->setIcon(QIcon::fromTheme("edit-closeBtn"));

    QHBoxLayout *mainLayout = new QHBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(tabBar, 1, Qt::AlignLeft);
    mainLayout->addWidget(hSplitBtn, 0, Qt::AlignRight);
    mainLayout->addWidget(vSplitBtn, 0, Qt::AlignRight);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);
}

void TabBarPrivate::initConnection()
{
    connect(tabBar, &DTabBar::currentChanged, this, &TabBarPrivate::onCurrentTabChanged);
    connect(tabBar, &DTabBar::tabCloseRequested, this, &TabBarPrivate::onTabColseRequested);
    connect(hSplitBtn, &DToolButton::clicked, this, [this] { emit q->spliterClicked(Qt::Horizontal); });
    connect(vSplitBtn, &DToolButton::clicked, this, [this] { emit q->spliterClicked(Qt::Vertical); });
    connect(closeBtn, &DToolButton::clicked, q, &TabBar::closeRequested);
}

void TabBarPrivate::onCurrentTabChanged(int index)
{
    const auto &file = q->indexFile(index);
    emit q->tabSwitched(file);
    editor.switchedFile(file);
}

void TabBarPrivate::onTabColseRequested(int index)
{
    const auto &file = q->indexFile(index);
    q->removeTab(file);
}

TabBar::TabBar(QWidget *parent)
    : QWidget(parent),
      d(new TabBarPrivate(this))
{
    d->initUI();
    d->initConnection();
}

void TabBar::setFileName(const QString &fileName)
{
    if (fileName.isEmpty() || !QFile::exists(fileName))
        return;

    int index = indexOf(fileName);
    if (-1 != index) {
        d->tabBar->setCurrentIndex(index);
        return;
    }

    QFileInfo info(fileName);
    index = d->tabBar->addTab(info.fileName());
    d->tabBar->setTabToolTip(index, fileName);
    d->tabBar->setCurrentIndex(index);
}

int TabBar::indexOf(const QString &fileName)
{
    int index = -1;
    for (int i = 0; i < d->tabBar->count(); ++i) {
        if (d->tabBar->tabToolTip(i) != fileName)
            continue;

        index = i;
        break;
    }

    return index;
}

QString TabBar::indexFile(int index) const
{
    return d->tabBar->tabToolTip(index);
}

int TabBar::tabCount() const
{
    return d->tabBar->count();
}

int TabBar::currentIndex() const
{
    return d->tabBar->currentIndex();
}

QString TabBar::currentFileName() const
{
    return indexFile(currentIndex());
}

void TabBar::setCurrentIndex(int index)
{
    d->tabBar->setCurrentIndex(index);
}

void TabBar::switchTab(const QString &fileName)
{
    int index = indexOf(fileName);
    if (-1 != index)
        d->tabBar->setCurrentIndex(index);
}

void TabBar::removeTab(const QString &fileName)
{
    int index = indexOf(fileName);
    if (-1 == index)
        return;

    QString text = d->tabBar->tabText(index);
    QFileInfo info(fileName);
    if (info.exists() && text.length() > 0 && text.at(0) == "*") {
        int ret = QMessageBox::question(this, QMessageBox::tr("Save Changes"),
                                        QMessageBox::tr("The file has unsaved changes, will save?"),
                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                        QMessageBox::Cancel);
        if (QMessageBox::Yes != ret && QMessageBox::No != ret) {
            return;
        } else if (QMessageBox::Yes == ret) {
            emit saveFileRequested(fileName);
        }
    }

    emit tabClosed(fileName);
    editor.fileClosed(fileName);
    d->tabBar->removeTab(index);
}

void TabBar::setCloseButtonVisible(bool visible)
{
    d->closeBtn->setVisible(visible);
}

void TabBar::setSplitButtonVisible(bool visible)
{
    d->hSplitBtn->setVisible(visible);
    d->vSplitBtn->setVisible(visible);
}

void TabBar::closeTab(int index)
{
    d->onTabColseRequested(index);
}

void TabBar::onFileChanged(const QString &fileName, bool isModified)
{
    int index = indexOf(fileName);
    if (-1 == index)
        return;

    QString changedFileName = QFileInfo(fileName).fileName();
    if (isModified)
        changedFileName.prepend("*");

    if (d->tabBar->tabText(index) != changedFileName)
        d->tabBar->setTabText(index, changedFileName);
}

void TabBar::onFileSaved(const QString &fileName)
{
    int index = indexOf(fileName);
    if (index == -1)
        return;

    auto text = d->tabBar->tabText(index);
    if (QFileInfo(fileName).fileName() == text)
        return;

    text = text.remove(0, 1);
    d->tabBar->setTabText(index, text);
}
