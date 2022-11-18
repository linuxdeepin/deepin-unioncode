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
#include "texteditkeeper.h"
#include "transceiver/codeeditorreceiver.h"
#include "common/common.h"

#include <QGridLayout>
#include <QFileInfo>
#include <QKeyEvent>

static TextEditTabWidget *ins{nullptr};

class TextEditTabWidgetPrivate
{
    friend class TextEditTabWidget;
    TextEditTabBar *tab = nullptr;
    QGridLayout *gridLayout = nullptr;
    QHash<QString, TextEdit*> textEdits;
    QHash<QString, TextEditTitleBar*> titleBars;
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

    if (!d->tab) {
        d->tab = new TextEditTabBar(this);
    }

    d->gridLayout->addWidget(d->tab);
    d->gridLayout->addWidget(&d->defaultEdit);
    this->setLayout(d->gridLayout);
    //d->defaultEdit.setCursor();

    setDefaultFileEdit();
    setFocusPolicy(Qt::ClickFocus);

    QObject::connect(DpfEventMiddleware::instance(),
                     QOverload<const newlsp::ProjectKey &, const QString &>::of(&DpfEventMiddleware::toOpenFile),
                     this, QOverload<const newlsp::ProjectKey &, const QString &>::of(&TextEditTabWidget::openFile));

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toRunFileLine,
                     this, &TextEditTabWidget::runningToLine);

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toRunClean,
                     this, &TextEditTabWidget::runningEnd);

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toDebugPointClean,
                     this, &TextEditTabWidget::debugPointClean);

    QObject::connect(DpfEventMiddleware::instance(),
                     QOverload<const newlsp::ProjectKey &, const QString &, int>::of(&DpfEventMiddleware::toJumpFileLine),
                     this, QOverload<const newlsp::ProjectKey &, const QString &, int>::of(&TextEditTabWidget::jumpToLine));

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toSetLineBackground,
                     this, &TextEditTabWidget::setLineBackground);

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toDelLineBackground,
                     this, &TextEditTabWidget::delLineBackground);

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toCleanLineBackground,
                     this, &TextEditTabWidget::cleanLineBackground);

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toSetAnnotation,
                     this, &TextEditTabWidget::setAnnotation);

    QObject::connect(DpfEventMiddleware::instance(), &DpfEventMiddleware::toCleanAnnotation,
                     this, &TextEditTabWidget::cleanAnnotation);

    QObject::connect(d->tab, &TextEditTabBar::fileSwitched,
                     this, &TextEditTabWidget::showFileEdit);

    QObject::connect(d->tab, &TextEditTabBar::fileClosed,
                     this, &TextEditTabWidget::removeFileEdit, Qt::QueuedConnection);

    QObject::connect(d->tab, &TextEditTabBar::fileClosed,
                     this, &TextEditTabWidget::removeFileStatusBar, Qt::QueuedConnection);

    QObject::connect(d->tab, &TextEditTabBar::close, this, [this](){
        emit signalEditClose();
    });

    QObject::connect(d->tab, &TextEditTabBar::horizontalSplit, this, [=](QString file){
        //d->defaultEdit
        emit signalEditSplit(Qt::Horizontal, file);
    });

//    QObject::connect(d->tab, &TextEditTabBar::verticalSplit, this, [=](){
//        emit signalEditSplit(Qt::Vertical);
//    });

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

    QObject::connect(edit, &TextEdit::signalFocusInChanged,
                     this, [=](){
        std::cerr << "TextEdit::focusInEvent" << std::endl;
        d->selFlag = true;
        emit signalFocusInChange();
        update();
    });

    QObject::connect(edit, &TextEdit::signalFocusOutChanged,
                     this, [=](){
        std::cerr << "TextEdit::focusOutEvent" << std::endl;
        d->selFlag = false;
        emit signalFocusOutChange();
        update();
    });

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
    showFileEdit(filePath);
}

void TextEditTabWidget::openFile(const newlsp::ProjectKey &key, const QString &filePath)
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

    QObject::connect(edit, &TextEdit::signalFocusInChanged,
                     this, [=](){
        std::cerr << "TextEdit::focusInEvent" << std::endl;
        d->selFlag = true;
        emit signalFocusInChange();
        update();
    });

    QObject::connect(edit, &TextEdit::signalFocusOutChanged,
                     this, [=](){
        std::cerr << "TextEdit::focusOutEvent" << std::endl;
        d->selFlag = false;
        emit signalFocusOutChange();
        update();
    });


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

void TextEditTabWidget::jumpToLine(const newlsp::ProjectKey &head, const QString &filePath, int line)
{
    auto edit = switchFileAndToOpen(head, filePath);

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

void TextEditTabWidget::setAnnotation(const QString &filePath, int line, const QString &text, int role)
{
    if (!d->gridLayout)
        return;

    auto edit = d->textEdits.value(filePath);

    if (!edit)
        return;

    edit->setAnnotation(line, text, role);
}

void TextEditTabWidget::cleanAnnotation(const QString &filePath)
{
    if (!d->gridLayout)
        return;

    auto edit = d->textEdits.value(filePath);

    if (!edit)
        return;

    edit->cleanAnnotation();
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

    edit->deleteLater();
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
        d->titleBars[file]->show();
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

TextEdit* TextEditTabWidget::switchFileAndToOpen(const newlsp::ProjectKey &head, const QString &filePath)
{
    auto edit = d->textEdits.value(filePath);
    if (edit) {
        d->tab->switchFile(filePath);
        showFileEdit(filePath);
    } else {
        openFile(head, filePath);
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
    d->selFlag = true;
    qInfo() << "TextEditTabWidget::focusInEvent" << this;
    emit signalFocusInChange();
    update();
}

void TextEditTabWidget::focusOutEvent(QFocusEvent *event)
{
    d->selFlag = false;
    qInfo() << "TextEditTabWidget::focusOutEvent" << this;
    emit signalFocusOutChange();
    update();
}

void TextEditTabWidget::paintEvent(QPaintEvent *event)
{
    if (d->selFlag) {
        QPainter painter(this);
        painter.save();
        painter.setPen(d->selColor);
        painter.drawRect(this->rect());
        painter.restore();
        // qInfo() << "setting custom color" << this;
    } else {
        if (!d->defColor.isValid()) {
            d->defColor = palette().background().color();
            d->selColor = QColor(d->defColor.red() + 20, d->defColor.green() + 20, d->defColor.blue() + 20, d->defColor.alpha());
            // qInfo() << "cache default color" << this;
        } else {
            // qInfo() << "recovery default color" << this;
            QPainter painter(this);
            painter.save();
            painter.setPen(d->defColor);
            painter.drawRect(this->rect());
            painter.restore();
        }
    }
}
