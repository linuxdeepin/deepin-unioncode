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
#include "textedittabwidget.h"
#include "textedittitlebar.h"
#include "textedittabbar.h"
#include "textedit.h"
#include "style/stylekeeper.h"
#include "transceiver/codeeditorreceiver.h"
#include "common/common.h"
#include "services/workspace/workspaceservice.h"

#include <QGridLayout>
#include <QFileInfo>

static TextEditTabWidget *ins{nullptr};

class TextEditTabWidgetPrivate
{
    friend class TextEditTabWidget;
    TextEditTabBar *tab = nullptr;
    QGridLayout *gridLayout = nullptr;
    QHash<QString, ScintillaEditExtern*> textEdits;
    QHash<QString, TextEditTitleBar*> titleBars;
    TextEdit defaultEdit;
    QString runningFilePathCache;
};

TextEditTabWidget::TextEditTabWidget(QWidget *parent)
    : QWidget(parent)
    , d(new TextEditTabWidgetPrivate)
{
    d->gridLayout = new QGridLayout(this);
    d->gridLayout->setSpacing(0);
    d->gridLayout->setMargin(0);

    if (!d->tab) {
        d->tab = new TextEditTabBar(this);
    }

    d->gridLayout->addWidget(d->tab);
    d->gridLayout->addWidget(&d->defaultEdit);
    this->setLayout(d->gridLayout);

    setDefaultFileEdit();

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toOpenFile,
                     this, &TextEditTabWidget::openFile);

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toRunFileLine,
                     this, &TextEditTabWidget::runningToLine);

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toRunClean,
                     this, &TextEditTabWidget::runningEnd);

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toDebugPointClean,
                     this, &TextEditTabWidget::debugPointClean);

    QObject::connect(d->tab, &TextEditTabBar::fileSwitched,
                     this, &TextEditTabWidget::showFileEdit);

    QObject::connect(d->tab, &TextEditTabBar::fileClosed,
                     this, &TextEditTabWidget::removeFileEdit, Qt::QueuedConnection);

    QObject::connect(d->tab, &TextEditTabBar::fileClosed,
                     this, &TextEditTabWidget::removeFileStatusBar, Qt::QueuedConnection);

    //    QObject::connect(Inotify::globalInstance(), &Inotify::deletedSelf, this, &TextEditTabWidget::fileDeleted);
    //    QObject::connect(Inotify::globalInstance(), &Inotify::movedSelf, this, &TextEditTabWidget::fileMoved);
    //    QObject::connect(Inotify::globalInstance(), &Inotify::modified, this, &TextEditTabWidget::fileModifyed);

}

TextEditTabWidget::~TextEditTabWidget()
{
    if (d) {
        if (d->tab) {
            delete d->tab;
            d->tab = nullptr;
        }
        auto itera = d->textEdits.begin();
        while (itera != d->textEdits.end()){
            delete itera.value(); // free instance
            itera = d->textEdits.erase(itera);
        }
        delete d; // free private
    }
}

TextEditTabWidget *TextEditTabWidget::instance()
{
    if (!ins)
        ins = new TextEditTabWidget;
    return ins;
}

void TextEditTabWidget::openFile(const QString &filePath, const QString &rootPath)
{
    QFileInfo info(filePath);
    if (!info.exists() || !d->tab)
        return;

    // can't add widget to much
    if (d->textEdits.keys().contains(info.filePath())) {
        d->tab->switchFile(filePath);
        return;
    }

    d->tab->setFile(filePath);


    TextEdit *edit = new TextEdit;

    QObject::connect(edit, &TextEdit::fileChanged, d->tab,
                     &TextEditTabBar::doFileChanged, Qt::UniqueConnection);

    QObject::connect(edit, &TextEdit::fileSaved, d->tab,
                     &TextEditTabBar::doFileSaved, Qt::UniqueConnection);

    edit->setRootPath(rootPath);
    edit->setFile(info.filePath());
    d->textEdits[filePath] = edit;

    // 添加监听
    Inotify::globalInstance()->addPath(info.filePath());
    // set display textedit
    d->gridLayout->addWidget(edit);

    if (!d->defaultEdit.isHidden())
        d->defaultEdit.hide();

    d->tab->switchFile(filePath);
    showFileEdit(filePath);
}

void TextEditTabWidget::closeFile(const QString &filePath)
{
    Inotify::globalInstance()->removePath(filePath);

    if (!d->tab)
        return;

    int index = d->tab->fileIndex(filePath);
    if (index >=0 && index < d->tab->count())
        emit d->tab->tabCloseRequested(index);
}

void TextEditTabWidget::jumpToLine(const QString &filePath, int line)
{
    auto edit = switchFileAndToOpen(filePath);

    if (edit) {
        edit->jumpToLine(line);
    }
}

void TextEditTabWidget::jumpToRange(const QString &filePath, const lsp::Range &range)
{
    auto edit = switchFileAndToOpen(filePath);

    if (edit) {
        auto set = StyleKeeper::create(edit->langueage());
        if (set.lsp) {
            auto start = set.lsp->getSciPosition(edit->docPointer(), range.start);
            auto end = set.lsp->getSciPosition(edit->docPointer(), range.end);
            edit->jumpToRange(start, end);
        }
    }
}

void TextEditTabWidget::runningToLine(const QString &filePath, int line)
{
    auto edit = switchFileAndToOpen(filePath);

    if (edit) {
        edit->jumpToLine(line);
        edit->runningToLine(line);
    }
}

void TextEditTabWidget::runningEnd()
{
    for (auto editor : d->textEdits) {
        editor->runningEnd();
    }
}

void TextEditTabWidget::debugPointClean()
{
    for (auto editor : d->textEdits) {
        editor->debugPointAllDelete();
    }
}

void TextEditTabWidget::replaceRange(const QString &filePath, const lsp::Range &range, const QString &text)
{
    auto editor = d->textEdits.value(filePath);
    if (editor) {
        auto set = StyleKeeper::create(editor->langueage());
        if (set.lsp) {
            auto start = set.lsp->getSciPosition(editor->docPointer(), range.start);
            auto end = set.lsp->getSciPosition(editor->docPointer(), range.end);
            editor->replaceRange(start, end, text);
        }
    } else { //直接更改磁盘数据
        using namespace dpfservice;
        auto &ctx = dpfInstance.serviceContext();
        WorkspaceService *workspace = ctx.service<WorkspaceService>(WorkspaceService::name());
        QStringList workspaceFolders;
        if (workspace && workspace->findWorkspace) {
            workspaceFolders = workspace->findWorkspace(filePath);
        }
        if (workspaceFolders.isEmpty()) {
            return;
        }
        else {
            if (range.start.line != range.end.line) {
                qCritical() << "Failed, Unknown error";
                abort();
            }
            QFile changeFile(filePath);
            QString cacheData;
            if (changeFile.open(QFile::ReadOnly)) {
                int i = 0;
                while (i != range.start.line) {
                    cacheData += changeFile.readLine();
                    i++;
                }
                QString changeLine = changeFile.readLine();
                int removeLength = range.end.character - range.start.character;
                changeLine = changeLine.replace(range.start.character, removeLength, text);
                cacheData += changeLine;
                QByteArray array = changeFile.readLine();
                while (!array.isEmpty()) {
                    cacheData += array;
                    array = changeFile.readLine();
                }
                changeFile.close();
            }

            if (changeFile.open(QFile::WriteOnly | QFile::Truncate)) {
                int writeCount = changeFile.write(cacheData.toLatin1());
                if (writeCount != cacheData.size()) {
                    qCritical() << "Failed, Unknown error";
                    abort();
                }
                changeFile.close();
            }
        }
    }
}

void TextEditTabWidget::setDefaultFileEdit()
{
    if (!d || !d->gridLayout)
        return;

    d->gridLayout->addWidget(&d->defaultEdit);
    d->defaultEdit.setEnabled(false);
    d->defaultEdit.show();
}

void TextEditTabWidget::hideFileEdit(const QString &file)
{
    if (!d->gridLayout)
        return;

    auto edit = d->textEdits.value(file);

    if (!edit)
        return;

    edit->hide();
}

void TextEditTabWidget::showFileEdit(const QString &file)
{
    if (!d->gridLayout)
        return;

    auto edit = d->textEdits.value(file);

    if (!edit)
        return;

    auto itera = d->textEdits.begin();
    while (itera != d->textEdits.end()){
        if (itera.key() != file){
            itera.value()->hide(); // hide other;
        }
        itera ++;
    }

    edit->show();
}

void TextEditTabWidget::hideFileStatusBar(const QString &file)
{
    auto statusBar = d->titleBars.value(file);
    statusBar->hide();
}

void TextEditTabWidget::showFileStatusBar(const QString &file)
{
    auto statusBar = d->titleBars.value(file);
    statusBar->show();
}

void TextEditTabWidget::removeFileStatusBar(const QString &file)
{
    auto statusBar = d->titleBars.value(file);
    if (!statusBar)
        return;

    delete statusBar;
    d->titleBars.remove(file);
}

void TextEditTabWidget::removeFileEdit(const QString &file)
{
    auto edit = d->textEdits.value(file);
    if (!edit)
        return;

    delete edit;
    d->textEdits.remove(file);

    if (d->textEdits.size() == 0)
        setDefaultFileEdit();
}

void TextEditTabWidget::removeFileTab(const QString &file)
{
    d->tab->removeTab(file);
}

void TextEditTabWidget::fileModifyed(const QString &file)
{
    if (!d->titleBars[file]) {
        d->titleBars[file] = TextEditTitleBar::changedReload(file);
    }
    auto editor = d->textEdits[file];
    if (editor && !editor->isHidden()) {
        d->gridLayout->addWidget(d->titleBars[file], 1, 0);
        d->titleBars[file]->show();
    }
}

void TextEditTabWidget::fileDeleted(const QString &file)
{
    Inotify::globalInstance()->removePath(file);
    QFileInfo info(file);
    if (info.exists()) {
        Inotify::globalInstance()->addPath(file);
        return fileModifyed(file);
    }
    qInfo() << "fileDeleted" << file;
}

void TextEditTabWidget::fileMoved(const QString &file)
{
    qInfo() << "fileMoved" << file;
}

ScintillaEditExtern* TextEditTabWidget::switchFileAndToOpen(const QString &filePath)
{
    auto edit = d->textEdits.value(filePath);
    if (edit) {
        d->tab->switchFile(filePath);
        showFileEdit(filePath);
    } else {
        openFile(filePath, "");
        for (auto editor : d->textEdits) {
            editor->runningEnd();
            if (editor->file() == filePath) {
                showFileEdit(filePath);
                edit = editor;
            }
        }
    }
    return edit;
}
