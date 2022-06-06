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
#include "scintillaeditextern.h"
#include "style/stylecolor.h"
#include "transceiver/sendevents.h"

#include "common/common.h"
#include "framework/framework.h"

#include <QApplication>
#include <QFile>
#include <QPoint>
#include <QRegularExpression>
#include <bitset>

class ScintillaEditExternPrivate
{
    friend class ScintillaEditExtern;
    ScintillaEditExternPrivate() {}
    bool isCtrlKeyPressed;
    bool isLeave;
    Scintilla::Position hoverPos = -1;
    QTimer hoverTimer;
    QTimer definitionHoverTimer;
    QString filePath;
    QString language;
    Head proHead;
    Scintilla::Position editInsertPostion = -1;
    int editInsertCount = 0;
};

ScintillaEditExtern::ScintillaEditExtern(QWidget *parent)
    : ScintillaEdit (parent)
    , d(new ScintillaEditExternPrivate)
{
    styleSetBack(STYLE_DEFAULT, StyleColor::color(QColor(22,22,22)));
}

ScintillaEditExtern::~ScintillaEditExtern()
{
    if (d) {
        delete d;
    }
}

QString ScintillaEditExtern::fileLanguage(const QString &path)
{
    using namespace support_file;
    return Language::id(path);
}

void ScintillaEditExtern::setFile(const QString &filePath)
{
    if (d->filePath == filePath) {
        return;
    } else {
        d->filePath = filePath;
    }

    QString text;
    QFile file(d->filePath);
    if (file.open(QFile::OpenModeFlag::ReadOnly)) {
        text = file.readAll();
        file.close();
    }
    setText(text.toUtf8());
    emptyUndoBuffer();
    setSavePoint();

    setMouseDwellTime(0);
    QObject::connect(this, &ScintillaEditExtern::marginClicked, this, &ScintillaEditExtern::sciMarginClicked, Qt::UniqueConnection);
    QObject::connect(this, &ScintillaEditExtern::modified, this, &ScintillaEditExtern::sciModified, Qt::UniqueConnection);
    QObject::connect(this, &ScintillaEditExtern::dwellStart, this, &ScintillaEditExtern::sciDwellStart, Qt::UniqueConnection);
    QObject::connect(this, &ScintillaEditExtern::dwellEnd, this, &ScintillaEditExtern::sciDwellEnd, Qt::UniqueConnection);
    QObject::connect(this, &ScintillaEditExtern::notify, this, &ScintillaEditExtern::sciNotify, Qt::UniqueConnection);
    QObject::connect(this, &ScintillaEditExtern::updateUi, this, &ScintillaEditExtern::sciUpdateUi, Qt::UniqueConnection);

    QObject::connect(&d->hoverTimer, &QTimer::timeout, &d->hoverTimer, [=](){
        emit this->hovered(d->hoverPos);
        d->hoverTimer.stop();
    }, Qt::UniqueConnection);

    QObject::connect(&d->definitionHoverTimer, &QTimer::timeout, &d->definitionHoverTimer, [=](){
        emit this->definitionHover(d->hoverPos);
        d->definitionHoverTimer.stop();
    }, Qt::UniqueConnection);
}

void ScintillaEditExtern::setFile(const QString &filePath, const Head &projectHead)
{
    d->proHead = projectHead;
    setFile(filePath);
}

void ScintillaEditExtern::updateFile()
{
    QString text;
    QFile file(d->filePath);
    if (file.open(QFile::OpenModeFlag::ReadOnly)) {
        text = file.readAll();
        file.close();
    }
    setText(text.toUtf8());
    emptyUndoBuffer();
    setSavePoint();
}

Head ScintillaEditExtern::projectHead()
{
    return d->proHead;
}

QString ScintillaEditExtern::file() const
{
    return d->filePath;
}

void ScintillaEditExtern::debugPointAllDelete()
{
    markerDeleteAll(StyleSci::Debug);
}

void ScintillaEditExtern::jumpToLine(int line)
{
    int lineOffSet = line - 1;
    int displayLines = linesOnScreen();
    if (displayLines > 0) {
        int offsetLines = displayLines / 2;
        if (lineOffSet > offsetLines) {
            setFirstVisibleLine(lineOffSet - offsetLines);
        }
    }
}

void ScintillaEditExtern::jumpToRange(Scintilla::Position start, Scintilla::Position end)
{
    jumpToLine(lineFromPosition(end));
    setSelectionStart(start);
    setSelectionEnd(end);
}

void ScintillaEditExtern::runningToLine(int line)
{
    int lineOffSet = line - 1;

    markerDeleteAll(StyleSci::Running);
    markerDeleteAll(StyleSci::RunningLineBackground);

    markerAdd(lineOffSet, StyleSci::Running);
    markerAdd(lineOffSet, StyleSci::RunningLineBackground);
}

void ScintillaEditExtern::runningEnd()
{
    markerDeleteAll(StyleSci::Running);
    markerDeleteAll(StyleSci::RunningLineBackground);
}

void ScintillaEditExtern::saveText()
{
    QFile file(d->filePath);
    if (!file.exists())
        return;

    if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        return;
    }
    Inotify::globalInstance()->addIgnorePath(d->filePath);
    file.write(textRange(0, length()));
    emit saved(d->filePath);
    file.close();
    Inotify::globalInstance()->removeIgnorePath(d->filePath);
}


void ScintillaEditExtern::saveAsText()
{
    QFile file(d->filePath);
    if (!file.open(QFile::ReadWrite | QFile::Text | QFile::Truncate)) {
        ContextDialog::ok("Can't save current: " + file.errorString());
        return;
    }
    Inotify::globalInstance()->addIgnorePath(d->filePath);
    file.write(textRange(0, length()));
    emit saved(d->filePath);
    file.close();
    Inotify::globalInstance()->removeIgnorePath(d->filePath);
}

bool ScintillaEditExtern::isLeave()
{
    return d->isLeave;
}

void ScintillaEditExtern::replaceRange(Scintilla::Position start,
                                       Scintilla::Position end, const QString &text)
{
    clearSelections();
    setSelectionStart(start);
    setSelectionEnd(end);
    replaceSel(text.toLatin1());
    emit replaceed(file(), start, end, text);
}

QPair<long int, long int> ScintillaEditExtern::findText(long int start, long int end, const QString &text)
{
    return ScintillaEdit::findText(STYLE_DEFAULT, text.toLatin1().data(), start, end);
}

void ScintillaEditExtern::findNext(const QString &srcText)
{
    long int currentPos = ScintillaEditExtern::currentPos();
    QPair<int, int> position = ScintillaEditExtern::findText(currentPos, length(), srcText.toLatin1().data());
    if (position.first > -1 && position.first != position.second) {
        ScintillaEditExtern::jumpToRange(position.first, position.second);
    }
}

void ScintillaEditExtern::replaceAll(const QString &srcText, const QString &destText)
{
    ScintillaEdit::searchAnchor();
    long int textLength = length();
    for (long int index = 0; index < textLength;) {
        QPair<int, int> position = ScintillaEditExtern::findText(index, textLength, srcText.toLatin1().data());
        if (position.first > -1 && position.first != position.second) {
            index = position.second;
            replaceRange(position.first, position.second, destText);
        }

        if (position.second >= textLength || index < 0)
            return;
    }
}

void ScintillaEditExtern::sciModified(Scintilla::ModificationFlags type, Scintilla::Position position,
                                      Scintilla::Position length, Scintilla::Position linesAdded,
                                      const QByteArray &text, Scintilla::Position line,
                                      Scintilla::FoldLevel foldNow, Scintilla::FoldLevel foldPrev)
{
    Q_UNUSED(position)
    Q_UNUSED(length)
    Q_UNUSED(linesAdded)
    Q_UNUSED(text)
    Q_UNUSED(line)
    Q_UNUSED(foldNow)
    Q_UNUSED(foldPrev)

    if (file().isEmpty()|| !QFile(file()).exists())
        return;

    if (bool(type & Scintilla::ModificationFlags::InsertText)) {
        textInserted(position, length, linesAdded, text, line);
    }

    if (bool(type & Scintilla::ModificationFlags::DeleteText)) {
        textDeleted(position, length, linesAdded, text, line);
    }
}

void ScintillaEditExtern::sciNotify(Scintilla::NotificationData *data)
{
    switch (data->nmhdr.code) {
    case Scintilla::Notification::IndicatorClick :
        emit indicClicked(data->position);
        break;
    case Scintilla::Notification::IndicatorRelease:
        emit indicReleased(data->position);
        break;
    default:
        break;
    }
}

void ScintillaEditExtern::sciUpdateUi(Scintilla::Update update)
{
    Q_UNUSED(update);
    if (d->hoverTimer.isActive()) {
        d->hoverTimer.stop();
    }
}

void ScintillaEditExtern::sciDwellStart(int x, int y)
{
    if (d->hoverPos == -1) {
        d->hoverPos = positionFromPoint(x, y); // cache position
        bool isKeyCtrl = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
        if (isKeyCtrl) {
            d->definitionHoverTimer.start(20);
        } else {
            d->hoverTimer.start(500); // 如果间隔较小，导致收发管道溢出最终程序崩溃
        }
    }
}

void ScintillaEditExtern::sciDwellEnd(int x, int y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    if (d->hoverPos != -1) {
        if (d->definitionHoverTimer.isActive()) {
            d->definitionHoverTimer.stop();
        }
        emit definitionHoverCleaned(d->hoverPos);
        if (d->hoverTimer.isActive()) {
            d->hoverTimer.stop();
        }
        emit hoverCleaned(d->hoverPos);
        d->hoverPos = -1; // clean cache postion
    }
}

void ScintillaEditExtern::keyReleaseEvent(QKeyEvent *event)
{
    return ScintillaEdit::keyReleaseEvent(event);
}

void ScintillaEditExtern::keyPressEvent(QKeyEvent *event)
{
    bool isKeyCtrl = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
    bool isKeyS = event->key() == Qt::Key_S;
    if (isKeyCtrl && isKeyS) {
        saveText();
    }
    return ScintillaEdit::keyPressEvent(event);
}

void ScintillaEditExtern::sciMarginClicked(Scintilla::Position position, Scintilla::KeyMod modifiers, int margin)
{
    Q_UNUSED(modifiers);

    sptr_t line = lineFromPosition(position);
    if (margin == StyleSci::Margin::LineNumber || margin == StyleSci::Margin::Runtime) {
        std::bitset<32> flags(markerGet(line));
        if (!flags[StyleSci::Debug]) {
            markerAdd(line, StyleSci::Debug);
            SendEvents::marginDebugPointAdd(file(), line + 1); //line begin 1 from debug point setting
        } else {
            markerDelete(line, StyleSci::Debug);
            SendEvents::marginDebugPointRemove(file(), line + 1); //line begin 1 from debug point setting
        }
    }
}

void ScintillaEditExtern::focusInEvent(QFocusEvent *event)
{
    return ScintillaEdit::focusInEvent(event);
}

void ScintillaEditExtern::focusOutEvent(QFocusEvent *event)
{
    callTipCancel(); //cancel hover;
    return ScintillaEdit::focusOutEvent(event);
}

void ScintillaEditExtern::contextMenuEvent(QContextMenuEvent *event)
{
    if (selectionStart() == selectionEnd()) {
        ScintillaEdit::contextMenuEvent(event);
    } else {
        emit selectionMenu(event);
    }
}

void ScintillaEditExtern::enterEvent(QEvent *event)
{
    d->isLeave = false;
    ScintillaEdit::enterEvent(event);
}

void ScintillaEditExtern::leaveEvent(QEvent *event)
{
    d->isLeave = true;
    ScintillaEdit::leaveEvent(event);
}
