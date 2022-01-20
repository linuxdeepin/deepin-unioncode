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
#include "editwidget.h"
#include "editfiletabwidget.h"
#include "editfilestatusbar.h"
#include "edittextwidget.h"
#include "codeeditorreceiver.h"
#include "common/inotify/inotify.h"

#include <QGridLayout>
#include <QFileInfo>

class EditWidgetPrivate
{
    friend class EditWidget;
    EditFileTabWidget *tab = nullptr;
    QGridLayout *gridLayout = nullptr;
    QHash<QString, EditTextWidget*> textEdits;
    QHash<QString, EditFileStatusBar*> statusBars;
    EditTextWidget defaultEdit;
    QString runningFilePathCache;
};

EditWidget::EditWidget(QWidget *parent)
    : QWidget(parent)
    , d(new EditWidgetPrivate)
{
    d->gridLayout = new QGridLayout(this);
    d->gridLayout->setSpacing(0);
    d->gridLayout->setMargin(0);

    if (!d->tab) {
        d->tab = new EditFileTabWidget(this);
    }

    d->gridLayout->addWidget(d->tab);
    d->gridLayout->addWidget(&d->defaultEdit);
    this->setLayout(d->gridLayout);

    setDefaultFileEdit();

    QObject::connect(d->tab, &EditFileTabWidget::currentChanged,
                     this, &EditWidget::showFileEdit);

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toOpenFile,
                     this, &EditWidget::openFile);

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toRunFileLine,
                     this, &EditWidget::runningToLine);

    QObject::connect(d->tab, &EditFileTabWidget::tabCloseRequested,
                     this, &EditWidget::removeFileEdit, Qt::QueuedConnection);

    QObject::connect(d->tab, &EditFileTabWidget::tabCloseRequested,
                     this, &EditWidget::removeFileTab, Qt::QueuedConnection);

    QObject::connect(Inotify::globalInstance(), &Inotify::deletedSelf, this, &EditWidget::fileDeleted);
    QObject::connect(Inotify::globalInstance(), &Inotify::movedSelf, this, &EditWidget::fileMoved);
    QObject::connect(Inotify::globalInstance(), &Inotify::modified, this, &EditWidget::fileModifyed);
}

EditWidget::~EditWidget()
{
    if (d) {
        if (d->tab) {
            delete d->tab;
            d->tab = nullptr;
        }
        auto itera = d->textEdits.begin();
        while (itera != d->textEdits.end()){
            delete itera.value(); // free instance
            *itera = nullptr; // set instance nullptr
            itera ++;
        }
        d->textEdits.clear(); // clear hash
        delete d; // free private
    }
}

int EditWidget::tabIndex(const QString &tabTooltip)
{
    int tabCount = d->tab->count();
    for (int index = 0; index < tabCount; index++) {
        if(d->tab->tabToolTip(index) == tabTooltip) {
            return index;
        }
    }
    return -1;
}

void EditWidget::openFile(const QString &filePath)
{
    QFileInfo info(filePath);
    if (!info.exists() || !d->tab)
        return;

    // can't add widget to much
    if (d->textEdits.keys().contains(info.filePath())) {
        d->tab->setCurrentIndex(tabIndex(info.filePath()));
        return;
    }

    int idx = d->tab->addTab(info.fileName());
    d->tab->setTabToolTip(idx, info.filePath());
    d->tab->setTabsClosable(true);
    d->tab->setCurrentIndex(idx);
    EditTextWidget *edit = new EditTextWidget;
    d->textEdits.insert(info.filePath(), edit);
    edit->setCurrentFile(info.filePath());
    // 添加监听
    Inotify::globalInstance()->addPath(info.filePath());
    // set display textedit
    d->gridLayout->addWidget(edit);

    if (!d->defaultEdit.isHidden())
        d->defaultEdit.hide();

    showFileEdit(idx);
}

void EditWidget::closeFile(const QString &filePath)
{
    Inotify::globalInstance()->removePath(filePath);

    if (!d->tab)
        return;
    int index = tabIndex(filePath);

    if (index >=0 && index < d->tab->count())
        emit d->tab->tabCloseRequested(index);
}

void EditWidget::runningToLine(const QString &filePath, int line)
{
    int fileTabIndex = tabIndex(filePath);
    if ( 0 <= fileTabIndex) {
        showFileEdit(fileTabIndex);
    } else {
        openFile(filePath);
    }

    for (auto editor : d->textEdits) {
        if (!editor) {
            qCritical() << "Error, Crashed from iteration hash";
            abort();
        }

        if (editor->currentFile() != filePath) {
            editor->runningEnd();
        } else {
            editor->runningToLine(line);
            showFileEdit(tabIndex(filePath));
        }
    }
}

void EditWidget::runningEnd()
{
    for (auto editor : d->textEdits) {
        if (!editor) {
            qCritical() << "Error, Crashed from iteration hash";
            abort();
        }
        editor->runningEnd();
    }
}

void EditWidget::setDefaultFileEdit()
{
    if (!d || !d->gridLayout)
        return;

    d->gridLayout->addWidget(&d->defaultEdit);
    d->defaultEdit.setEnabled(false);
    d->defaultEdit.show();
}

void EditWidget::hideFileEdit(int tabIndex)
{
    if (!d->gridLayout)
        return;

    QString filePath = d->tab->tabToolTip(tabIndex);
    auto edit = d->textEdits.value(filePath);

    if (!edit)
        return;

    edit->hide();
}

void EditWidget::showFileEdit(int tabIndex)
{
    if (!d->gridLayout)
        return;

    QString filePath = d->tab->tabToolTip(tabIndex);
    auto edit = d->textEdits.value(filePath);

    if (!edit)
        return;

    auto itera = d->textEdits.begin();
    while (itera != d->textEdits.end()){
        if (itera.key() != d->tab->tabToolTip(tabIndex)){
            itera.value()->hide(); // hide other;
        }
        itera ++;
    }

    edit->show();
}

void EditWidget::hideFileStatusBar(int tabIndex)
{
    QString filePath = d->tab->tabToolTip(tabIndex);
    auto statusBar = d->statusBars.value(filePath);
    statusBar->hide();
}

void EditWidget::showFileStatusBar(int tabIndex)
{
    QString filePath = d->tab->tabToolTip(tabIndex);
    auto statusBar = d->statusBars.value(filePath);
    statusBar->show();
}

void EditWidget::removeFileStatusBar(int tabIndex)
{
    QString filePath = d->tab->tabToolTip(tabIndex);
    auto statusBar = d->statusBars.value(filePath);
    if (!statusBar)
        return;

    delete statusBar;
    d->statusBars.remove(filePath);
}

void EditWidget::removeFileEdit(int tabIndex)
{
    QString filePath = d->tab->tabToolTip(tabIndex);
    auto edit = d->textEdits.value(filePath);
    if (!edit)
        return;

    delete edit;
    d->textEdits.remove(filePath);

    if (d->textEdits.size() == 0)
        setDefaultFileEdit();
}

void EditWidget::removeFileTab(int tabIndex)
{
    if (!d->tab)
        return;

    d->tab->removeTab(tabIndex);
}

void EditWidget::fileModifyed(const QString &file)
{
    if (!d->statusBars[file]) {
        d->statusBars[file] = EditFileStatusBar::changedReload(file);
    }
    auto editor = d->textEdits[file];
    if (editor && !editor->isHidden()) {
        d->gridLayout->addWidget(d->statusBars[file], 1, 0);
        d->statusBars[file]->show();
    }
}

void EditWidget::fileDeleted(const QString &file)
{
    Inotify::globalInstance()->removePath(file);
    QFileInfo info(file);
    if (info.exists()) {
        Inotify::globalInstance()->addPath(file);
        return fileModifyed(file);
    }
    qInfo() << "fileDeleted" << file;
}

void EditWidget::fileMoved(const QString &file)
{
    qInfo() << "fileMoved" << file;
}
