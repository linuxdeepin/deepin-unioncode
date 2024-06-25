// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabbar.h"
#include "private/tabbar_p.h"

#include "common/util/eventdefinitions.h"

#include <DMenu>
#include <DDialog>
#include <DDesktopServices>
#include <DGuiApplicationHelper>

#include <QSignalBlocker>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QClipboard>
#include <QGuiApplication>

DWIDGET_USE_NAMESPACE

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
    tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
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

    updateBackgroundColor();
}

void TabBarPrivate::updateBackgroundColor()
{
    q->setAutoFillBackground(true);
    auto p = q->palette();
    if (Dtk::Gui::DGuiApplicationHelper::instance()->themeType() == Dtk::Gui::DGuiApplicationHelper::LightType)
        p.setColor(QPalette::Window, QColor(225, 225, 225));
    else
        p.setColor(QPalette::Window, QColor(47, 47, 47));
    q->setPalette(p);
}

bool TabBarPrivate::isModified(int index) const
{
    QString text = tabBar->tabText(index);
    return text.length() > 0 && text.at(0) == "*";
}

int TabBarPrivate::showConfirmDialog(const QString &filePath)
{
    DDialog dialog(q);
    dialog.setWindowTitle(tr("Save Changes"));
    dialog.setIcon(QIcon::fromTheme("dialog-warning"));
    dialog.setMessage(tr("The file %1 has unsaved changes, will save?").arg(QFileInfo(filePath).fileName()));
    dialog.addButton(tr("Save", "button"), true, DDialog::ButtonRecommend);
    dialog.addButton(tr("Do Not Save", "button"));
    dialog.addButton(tr("Cancel", "button"));

    return dialog.exec();
}

void TabBarPrivate::closeAllTab(const QStringList &exceptList)
{
    QStringList tabList;
    for (int i = 0; i < tabBar->count(); ++i) {
        auto file = tabBar->tabToolTip(i);
        if (exceptList.contains(file))
            continue;

        if (isModified(i)) {
            int ret = showConfirmDialog(file);
            if (ret == 0)   // save
                emit q->saveFileRequested(file);
            else if (ret == 2 || ret == -1)   // cancel or close
                return;
        }

        tabList << file;
    }

    for (const auto &tab : tabList)
        q->removeTab(tab, true);
}

void TabBarPrivate::initConnection()
{
    connect(tabBar, &DTabBar::currentChanged, this, &TabBarPrivate::onCurrentTabChanged);
    connect(tabBar, &DTabBar::tabCloseRequested, this, &TabBarPrivate::onTabColseRequested);
    connect(tabBar, &DTabBar::customContextMenuRequested, this, &TabBarPrivate::showMenu);
    connect(hSplitBtn, &DToolButton::clicked, this, [this] { emit q->spliterClicked(Qt::Horizontal); });
    connect(vSplitBtn, &DToolButton::clicked, this, [this] { emit q->spliterClicked(Qt::Vertical); });
    connect(closeBtn, &DToolButton::clicked, q, &TabBar::closeRequested);
    connect(Dtk::Gui::DGuiApplicationHelper::instance(), &Dtk::Gui::DGuiApplicationHelper::themeTypeChanged, this, &TabBarPrivate::updateBackgroundColor);
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

void TabBarPrivate::showMenu(QPoint pos)
{
    int curIndex = tabBar->tabAt(pos);
    DMenu menu;

    menu.addAction(tr("Copy File Path"), [=]() {
        auto file = tabBar->tabToolTip(curIndex);
        QGuiApplication::clipboard()->setText(file);
    });
    menu.addAction(tr("Copy File Name"), [=]() {
        auto file = tabBar->tabToolTip(curIndex);
        auto fileName = QFileInfo(file).fileName();
        QGuiApplication::clipboard()->setText(fileName);
    });

    menu.addSeparator();
    menu.addAction(tr("Close This File"), [=]() {
        auto file = tabBar->tabToolTip(curIndex);
        q->removeTab(file);
    });
    menu.addAction(tr("Close All Files"), [=]() {
        closeAllTab({});
    });
    menu.addAction(tr("Close All Files Except This"), [=]() {
        auto curFile = tabBar->tabToolTip(curIndex);
        QStringList exceptList { curFile };
        closeAllTab(exceptList);
    });

    menu.addSeparator();
    menu.addAction(tr("Open File Location"), [=]() {
        auto file = tabBar->tabToolTip(curIndex);
        DDesktopServices::showFileItem(file);
    });

    menu.exec(QCursor::pos());
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

    QSignalBlocker block(d->tabBar);
    QFileInfo info(fileName);
    index = d->tabBar->addTab(info.fileName());
    d->tabBar->setTabToolTip(index, fileName);
    d->tabBar->setCurrentIndex(index);

    emit tabSwitched(fileName);
    editor.switchedFile(fileName);
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

void TabBar::removeTab(const QString &fileName, bool silent)
{
    int index = indexOf(fileName);
    if (-1 == index)
        return;

    QFileInfo info(fileName);
    if (!silent && info.exists() && d->isModified(index)) {
        int ret = d->showConfirmDialog(fileName);
        if (ret == 0)   // save
            emit saveFileRequested(fileName);
        else if (ret == 2 || ret == -1)   // cancel or close
            return;
    }

    emit tabClosed(fileName);
    editor.fileClosed(fileName);
    d->tabBar->removeTab(index);
    editor.switchedFile(this->currentFileName());
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
