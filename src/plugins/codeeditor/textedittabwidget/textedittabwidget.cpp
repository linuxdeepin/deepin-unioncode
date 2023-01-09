/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             hongjinchuan<hongjinchuan@uniontech.com>
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
#include "style/lspclientkeeper.h"
#include "textedittabwidget.h"
#include "textedittitlebar.h"
#include "textedittabbar.h"
#include "textedit.h"
#include "mainframe/texteditkeeper.h"
#include "transceiver/codeeditorreceiver.h"
#include "codelens/codelens.h"
#include "common/common.h"
#include <QGridLayout>
#include <QFileInfo>
#include <QKeyEvent>

static TextEditTabWidget *ins{nullptr};

class TextEditTabWidgetPrivate
{
    friend class TextEditTabWidget;
    TextEditTabBar *tab {nullptr};
    QGridLayout *gridLayout {nullptr};
    QHash<QString, TextEdit*> textEdits{};
    QHash<QString, TextEditTitleBar*> titleBars{};
    QHash<QString, bool> textEditAutoReloadFlags;
    TextEdit defaultEdit;
    QString runningFilePathCache;
    bool selFlag = false;
    QColor selColor{};
    QColor defColor{};
};

TextEditTabWidget::TextEditTabWidget(QWidget *parent)
    : QWidget(parent)
    , d(new TextEditTabWidgetPrivate)
{
    d->gridLayout = new QGridLayout(this);
    d->gridLayout->setSpacing(0);
    d->gridLayout->setMargin(4);

    d->tab = new TextEditTabBar(this);
    d->gridLayout->addWidget(d->tab);
    d->gridLayout->addWidget(&d->defaultEdit);
    this->setLayout(d->gridLayout);
    this->setAcceptDrops(true);

    setDefaultFileEdit();
    setFocusPolicy(Qt::ClickFocus);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                     this, &TextEditTabWidget::openFileWithKey);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toRunFileLineWithKey,
                     this, &TextEditTabWidget::runningToLineWithKey);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toRunClean,
                     this, &TextEditTabWidget::runningEnd);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toDebugPointClean,
                     this, &TextEditTabWidget::debugPointClean);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toJumpFileLineWithKey,
                     this, &TextEditTabWidget::jumpToLineWithKey);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toSetLineBackground,
                     this, &TextEditTabWidget::setLineBackground);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toDelLineBackground,
                     this, &TextEditTabWidget::delLineBackground);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toCleanLineBackground,
                     this, &TextEditTabWidget::cleanLineBackground);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toSetAnnotation,
                     this, &TextEditTabWidget::setAnnotation);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toCleanAnnotation,
                     this, &TextEditTabWidget::cleanAnnotation);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toCleanAllAnnotation,
                     this, &TextEditTabWidget::cleanAllAnnotation);

    QObject::connect(d->tab, &TextEditTabBar::fileSwitched,
                     this, &TextEditTabWidget::showFileEdit, Qt::QueuedConnection);

    QObject::connect(d->tab, &TextEditTabBar::fileClosed,
                     this, &TextEditTabWidget::removeFileEdit, Qt::QueuedConnection);

    QObject::connect(d->tab, &TextEditTabBar::fileClosed,
                     this, &TextEditTabWidget::removeFileStatusBar, Qt::QueuedConnection);

    QObject::connect(d->tab, &TextEditTabBar::closeClicked, this, &TextEditTabWidget::closed);

    QObject::connect(d->tab, &TextEditTabBar::splitClicked, this, [=](Qt::Orientation ori) {
        QString currSelFile = d->tab->currentFile();
        newlsp::ProjectKey key = {};
        if(d->textEdits[currSelFile] && !currSelFile.isEmpty()) {
            key = d->textEdits[currSelFile]->projectKey();
        }
        emit splitClicked(ori, key, currSelFile);
    });

    QObject::connect(Inotify::globalInstance(), &Inotify::deletedSelf,
                     this, &TextEditTabWidget::fileDeleted, Qt::QueuedConnection);
    QObject::connect(Inotify::globalInstance(), &Inotify::movedSelf,
                     this, &TextEditTabWidget::fileMoved, Qt::QueuedConnection);
    QObject::connect(Inotify::globalInstance(), &Inotify::modified,
                     this, &TextEditTabWidget::fileModifyed, Qt::QueuedConnection);
}

TextEditTabWidget::~TextEditTabWidget()
{
    if (d) {
        if (d->tab) {
            delete d->tab;
            d->tab = nullptr;
        }

        auto editsItera = d->textEdits.begin();
        while (editsItera != d->textEdits.end()) {
            delete editsItera.value(); // free instance
            editsItera = d->textEdits.erase(editsItera);
        }

        auto titleItera = d->titleBars.begin();
        while (titleItera != d->titleBars.end()) {
            delete titleItera.value();
            titleItera = d->titleBars.erase(titleItera);
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

void TextEditTabWidget::openFile(const QString &filePath)
{
    QFileInfo info(filePath);
    if (!info.exists() || !d->tab )
        return;
    // can't add widget to much
    if (d->textEdits.keys().contains(info.filePath())) {
        d->tab->switchFile(filePath);
        return;
    }

    d->tab->setFile(filePath);


    TextEdit *edit = new TextEdit;

    QObject::connect(edit, &TextEdit::focusChanged,
                     this, &TextEditTabWidget::selectSelf);

    QObject::connect(edit, &TextEdit::fileChanged, d->tab,
                     &TextEditTabBar::doFileChanged, Qt::UniqueConnection);

    QObject::connect(edit, &TextEdit::fileSaved, d->tab,
                     &TextEditTabBar::doFileSaved, Qt::UniqueConnection);

    edit->setFile(info.filePath());
    d->textEdits[filePath] = edit;

    // 添加监听
    Inotify::globalInstance()->addPath(info.filePath());
    // set display textedit
    d->gridLayout->addWidget(edit);

    if (!d->defaultEdit.isHidden())
        d->defaultEdit.hide();

    d->tab->switchFile(filePath);

    d->textEditAutoReloadFlags[filePath] = false;

    showFileEdit(filePath);
    emit sigOpenFile();
}

void TextEditTabWidget::openFileWithKey(const newlsp::ProjectKey &key, const QString &filePath)
{
    QFileInfo info(filePath);
    if (!info.exists() || !d->tab )
        return;
    // can't add widget to much
    if (d->textEdits.keys().contains(info.filePath())) {
        d->tab->switchFile(filePath);
        return;
    }

    d->tab->setFile(filePath);

    newlsp::Client *client = LSPClientKeeper::instance()->get(key);
    if (!client) {
        client = LSPClientKeeper::instance()->get(key);
    }

    // 全局rename操作
    QObject::connect(client, QOverload<const newlsp::WorkspaceEdit&>::of(&newlsp::Client::renameRes),
                     this, &TextEditTabWidget::doRenameReplace, Qt::UniqueConnection);

    // 使用取出适用的编辑器
    TextEdit *edit  = TextEditKeeper::create(TextEdit::fileLanguage(filePath));

    QObject::connect(edit, &TextEdit::fileChanged, d->tab,
                     &TextEditTabBar::doFileChanged, Qt::UniqueConnection);

    QObject::connect(edit, &TextEdit::fileSaved, d->tab,
                     &TextEditTabBar::doFileSaved, Qt::UniqueConnection);

    QObject::connect(d->tab, &TextEditTabBar::saveFile,
                     this, &TextEditTabWidget::saveEditFile,
                     Qt::UniqueConnection);

    if (edit){
        edit->setProjectKey(key);
        edit->setFile(info.filePath());
    } else {
        edit = new TextEdit();
        edit->setProjectKey(key);
        edit->setFile(info.filePath());
    }

    QObject::connect(edit, &TextEdit::focusChanged,
                     this, &TextEditTabWidget::selectSelf);

    d->textEdits[filePath] = edit;
    // 添加监听
    Inotify::globalInstance()->addPath(info.filePath());
    // set display textedit
    d->gridLayout->addWidget(edit);

    if (!d->defaultEdit.isHidden())
        d->defaultEdit.hide();

    d->tab->switchFile(filePath);

    d->textEditAutoReloadFlags[filePath] = false;

    showFileEdit(filePath);
    emit sigOpenFile();
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

void TextEditTabWidget::jumpToLineWithKey(const newlsp::ProjectKey &key, const QString &filePath, int line)
{
    auto edit = switchFileAndToOpen(key, filePath);

    if (edit) {
        edit->jumpToLine(line);
    }
}

void TextEditTabWidget::jumpToLine(const QString &filePath, int line)
{
    auto edit = switchFileAndToOpen(filePath);

    if (edit) {
        edit->jumpToLine(line);
    }
}

void TextEditTabWidget::jumpToRange(const QString &filePath, const newlsp::Range &range)
{
    auto edit = switchFileAndToOpen(filePath);

    if (edit) {
        auto styleLsp = edit->getStyleLsp();
        if (styleLsp) {
            auto start = styleLsp->getSciPosition(edit->docPointer(), range.start);
            auto end = styleLsp->getSciPosition(edit->docPointer(), range.end);
            edit->jumpToRange(start, end);
        }
    }
}

void TextEditTabWidget::runningToLineWithKey(const newlsp::ProjectKey &key, const QString &filePath, int line)
{
    auto edit = switchFileAndToOpen(key, filePath);

    if (edit) {
        edit->jumpToLine(line);
        edit->runningToLine(line);
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
    for (auto edit : d->textEdits) {
        edit->runningEnd();
    }
}

void TextEditTabWidget::addDebugPoint(const QString &filePath, int line)
{
    for (auto edit : d->textEdits) {
        if (filePath == edit->file()) {
            edit->addDebugPoint(line);
        }
    }
}

void TextEditTabWidget::removeDebugPoint(const QString &filePath, int line)
{
    for (auto edit : d->textEdits) {
        if (filePath == edit->file()) {
            edit->removeDebugPoint(line);
        }
    }
}

void TextEditTabWidget::debugPointClean()
{
    for (auto edit : d->textEdits) {
        edit->debugPointAllDelete();
    }
}

void TextEditTabWidget::replaceRange(const QString &filePath, const newlsp::Range &range, const QString &text)
{
    auto edit = d->textEdits.value(filePath);
    if (edit) {
        auto styleLsp = edit->getStyleLsp();
        if (styleLsp) {
            auto start = styleLsp->getSciPosition(edit->docPointer(), range.start);
            auto end = styleLsp->getSciPosition(edit->docPointer(), range.end);
            edit->replaceRange(start, end, text);
        }
    } else { //直接更改磁盘数据
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

void TextEditTabWidget::setLineBackground(const QString &filePath, int line, const QColor &color)
{
    if (!d->gridLayout)
        return;

    auto edit = d->textEdits.value(filePath);

    if (!edit)
        return;

    edit->setLineBackground(line, color);
}

void TextEditTabWidget::delLineBackground(const QString &filePath, int line)
{
    if (!d->gridLayout)
        return;

    auto edit = d->textEdits.value(filePath);

    if (!edit)
        return;

    edit->delLineBackground(line);
}

void TextEditTabWidget::cleanLineBackground(const QString &filePath)
{
    if (!d->gridLayout)
        return;

    auto edit = d->textEdits.value(filePath);

    if (!edit)
        return;

    edit->cleanLineBackground();
}

void TextEditTabWidget::setAnnotation(const QString &filePath, int line,
                                      const QString &title, const AnnotationInfo &info)
{
    if (!d->gridLayout)
        return;

    auto edit = d->textEdits.value(filePath);

    if (!edit)
        return;

    edit->setAnnotation(line, title, info);
}

void TextEditTabWidget::cleanAnnotation(const QString &filePath, const QString &title)
{
    if (!d->gridLayout)
        return;

    auto edit = d->textEdits.value(filePath);

    if (!edit)
        return;

    edit->cleanAnnotation(title);
}

void TextEditTabWidget::cleanAllAnnotation(const QString &title)
{
    for (auto filePath : d->textEdits.keys()) {
        cleanAnnotation(filePath, title);
    }
}

void TextEditTabWidget::selectSelf(bool state)
{
    d->selFlag = state;
    emit selected(state);
    update();
}

void TextEditTabWidget::setModifiedAutoReload(const QString &filePath, bool flag)
{
    d->textEditAutoReloadFlags[filePath] = flag;
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
        if (itera.key() != file && itera.value()){
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
    if (d->titleBars.contains(file)) {
        auto statusBar = d->titleBars.value(file);
        if (statusBar)
            statusBar->show();
    }
}

void TextEditTabWidget::removeFileStatusBar(const QString &file)
{
    if (d->textEditAutoReloadFlags.contains(file))
        d->textEditAutoReloadFlags.remove(file);

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

    edit->fileClosed(file);
    edit->deleteLater();
    d->textEdits.remove(file);

    if (d->textEdits.size() == 0) {
        setDefaultFileEdit();
        emit closeWidget();
    }

}

void TextEditTabWidget::removeFileTab(const QString &file)
{
    d->tab->removeTab(file);
}

void TextEditTabWidget::fileModifyed(const QString &file)
{
    auto edit = d->textEdits[file];
    if (edit && !edit->isHidden() && !edit->isSaveText()) {

        if (!d->titleBars[file]) {
            d->titleBars[file] = TextEditTitleBar::changedReload(file);
            QObject::connect(d->titleBars[file], &TextEditTitleBar::reloadfile, [=](){
                if(d->titleBars.contains(file) && d->textEdits.contains(file)) {
                    d->textEdits[file]->updateFile();
                }
            });
        }

        d->gridLayout->addWidget(d->titleBars[file], 1, 0);
        if (d->textEditAutoReloadFlags.contains(file) && d->textEditAutoReloadFlags[file]) {
            d->titleBars[file]->reloadfile();
        } else {
            d->titleBars[file]->show();
        }
    }

    // 100 ms 内多次出发变动将忽略
    QTimer::singleShot(100, [=](){edit->cleanIsSaveText();});
}

void TextEditTabWidget::fileDeleted(const QString &file)
{
    detectFile(file);
}

void TextEditTabWidget::fileMoved(const QString &file)
{
    detectFile(file);
}

void TextEditTabWidget::detectFile(const QString &file)
{
    QFileInfo info(file);
    if (info.exists()) {
        fileModifyed(file);
        Inotify::globalInstance()->addPath(file);
    } else {
        Inotify::globalInstance()->removePath(file);
        handleDeletedFile(file);
    }
}

void TextEditTabWidget::setCloseButtonVisible(bool flag)
{
    d->tab->setCloseButtonVisible(flag);
}

void TextEditTabWidget::setSplitButtonVisible(bool flag)
{
    d->tab->setSplitButtonVisible(flag);
}

void TextEditTabWidget::handleDeletedFile(const QString &file)
{
    int ret = QMessageBox::question(this, QMessageBox::tr("File Has Been Removed"),
                                    QMessageBox::tr("The file has been removed, Do you want to save it?"),
                                    QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard);
    if (QMessageBox::Save == ret) {
        TextEdit* edit = d->textEdits.value(file);
        if (edit) {
            edit->saveAsText();
            Inotify::globalInstance()->addPath(file);
        }
    } else {
        closeFile(file);
    }
}

void TextEditTabWidget::doRenameReplace(const newlsp::WorkspaceEdit &renameResult)
{
    if (renameResult.changes) {
        auto changes = renameResult.changes;
        auto itera = changes->begin();
        while (itera != changes->end()) {
            for (auto edit : itera->second) {
                QString filePath = QUrl(QString::fromStdString(itera->first)).toLocalFile();
                QString newText = QString::fromStdString(edit.newText);
                replaceRange(filePath, edit.range, newText);
            }
            itera ++;
        }
    }
    if (renameResult.documentChanges) {
        if (newlsp::any_contrast<std::vector<newlsp::TextDocumentEdit>>(renameResult.documentChanges.value())) {
            std::vector<newlsp::TextDocumentEdit> documentChanges
                    = std::any_cast<std::vector<newlsp::TextDocumentEdit>>(renameResult.documentChanges.value());
            for (auto documentChange : documentChanges) {
                QString filePath = QUrl(QString::fromStdString(documentChange.textDocument.uri)).toLocalFile();
                if (!std::vector<newlsp::TextEdit>(documentChange.edits).empty()) {
                    auto edits = std::vector<newlsp::TextEdit>(documentChange.edits);
                    for (auto edit : edits) {
                        QString newText = QString::fromStdString(edit.newText);
                        replaceRange(filePath, edit.range, newText);
                    }
                } else if (!std::vector<newlsp::AnnotatedTextEdit>(documentChange.edits).empty()) {
                    auto edits = std::vector<newlsp::AnnotatedTextEdit>(documentChange.edits);
                    for (auto edit : edits) {
                        QString newText = QString::fromStdString(edit.newText);
                        replaceRange(filePath, edit.range, newText);
                    }
                }
            }
        }
    }
}

TextEdit* TextEditTabWidget::switchFileAndToOpen(const newlsp::ProjectKey &key, const QString &filePath)
{
    auto edit = d->textEdits.value(filePath);
    if (edit) {
        d->tab->switchFile(filePath);
        showFileEdit(filePath);
    } else {
        openFileWithKey(key, filePath);
        for (auto textEdit : d->textEdits.values()) {
            textEdit->runningEnd();
            if (textEdit->file() == filePath) {
                showFileEdit(filePath);
                edit = textEdit;
            }
        }
    }
    return edit;
}

TextEdit* TextEditTabWidget::switchFileAndToOpen(const QString &filePath)
{
    auto edit = d->textEdits.value(filePath);
    if (edit) {
        d->tab->switchFile(filePath);
        showFileEdit(filePath);
    } else {
        openFile(filePath);
        for (auto textEdit : d->textEdits.values()) {
            textEdit->runningEnd();
            if (textEdit->file() == filePath) {
                showFileEdit(filePath);
                edit = textEdit;
            }
        }
    }

    showFileStatusBar(filePath);
    return edit;
}

void TextEditTabWidget::saveEditFile(const QString &file)
{
    TextEdit* edit = d->textEdits.value(file);
    if (edit) {
        edit->saveText();
    }
}

void TextEditTabWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::AltModifier) {
        int idx = d->tab->currentIndex();
        int count = d->tab->count();
        if (count > 0 && idx > -1) {
            if (event->key() == Qt::Key_Left) {
                d->tab->setCurrentIndex((idx - 1 + count) % count);
                setFocus();
            } else if (event->key() == Qt::Key_Right) {
                d->tab->setCurrentIndex((idx + 1) % count);
                setFocus();
            }
        }
    }
    return QWidget::keyPressEvent(event);
}

void TextEditTabWidget::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    this->selectSelf(true);
}

void TextEditTabWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    this->selectSelf(false);
}

void TextEditTabWidget::paintEvent(QPaintEvent *event)
{
    if (d->selFlag) {
        QPainter painter(this);
        painter.save();
        painter.setPen(d->selColor);
        painter.drawRect(this->rect());
        painter.restore();
    } else {
        if (!d->defColor.isValid()) {
            d->defColor = palette().background().color();
            d->selColor = QColor(d->defColor.red() + 20, d->defColor.green() + 20, d->defColor.blue() + 20, d->defColor.alpha());
        } else {
            QPainter painter(this);
            painter.save();
            painter.setPen(d->defColor);
            painter.drawRect(this->rect());
            painter.restore();
        }
    }
}

void TextEditTabWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void TextEditTabWidget::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if(mimeData->hasUrls()) {
        QList<QUrl>urlList = mimeData->urls();
        QString fileName = urlList.at(0).toLocalFile();
        if(!fileName.isEmpty()) {
            editor.openFile(fileName);
        }
    }
}
