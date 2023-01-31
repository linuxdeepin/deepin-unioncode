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
#include "transceiver/codeeditorreceiver.h"

#include "common/common.h"
#include "framework/framework.h"

#include <QApplication>
#include <QFile>
#include <QPoint>
#include <QRegularExpression>
#include <QScrollBar>

#include <bitset>

class ScintillaEditExternPrivate
{
    friend class ScintillaEditExtern;
    ScintillaEditExternPrivate() {}
    bool isCtrlKeyPressed;
    bool isLeave;
    bool isSaveText = false;
    Scintilla::Position hoverPos = -1;
    QTimer hoverTimer;
    QTimer definitionHoverTimer;
    QString filePath;
    QString language;
    newlsp::ProjectKey proKey;
    Scintilla::Position editInsertPostion = -1;
    int editInsertCount = 0;
    QHash<int, QList<AnnotationInfo>> lineAnnotations;
    QHash<QString, decltype (lineAnnotations)> moduleAnnotations;
};

ScintillaEditExtern::ScintillaEditExtern(QWidget *parent)
    : ScintillaEdit (parent)
    , d(new ScintillaEditExternPrivate)
{
    setTabWidth(4);
    setIndentationGuides(SC_IV_LOOKBOTH);
    styleSetBack(STYLE_DEFAULT, StyleColor::color(QColor(43,43,43)));
    for (int i = 0; i < KEYWORDSET_MAX; i ++) {
        styleSetFore(i, StyleColor::color(QColor(0xdd, 0xdd, 0xdd)));
        styleSetBack(i,  StyleColor::color(QColor(43,43,43)));
    }
    setCaretFore(StyleColor::color(QColor(255,255,255)));
    horizontalScrollBar()->setVisible(false);
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

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toSearchText, this, &ScintillaEditExtern::find);
    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toReplaceText, this, &ScintillaEditExtern::replace);

    QObject::connect(&d->hoverTimer, &QTimer::timeout, &d->hoverTimer, [=](){
        emit this->hovered(d->hoverPos);
        d->hoverTimer.stop();
    }, Qt::UniqueConnection);

    QObject::connect(&d->definitionHoverTimer, &QTimer::timeout, &d->definitionHoverTimer, [=](){
        emit this->definitionHover(d->hoverPos);
        d->definitionHoverTimer.stop();
    }, Qt::UniqueConnection);
    horizontalScrollBar()->setVisible(true);
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

QString ScintillaEditExtern::file() const
{
    return d->filePath;
}

void ScintillaEditExtern::setProjectKey(const newlsp::ProjectKey &key)
{
    d->proKey = key;
}

QString ScintillaEditExtern::workspace() const
{
    return QString::fromStdString(d->proKey.workspace);
}

void ScintillaEditExtern::addDebugPoint(int line)
{
    markerAdd(line, StyleSci::Debug);
}

void ScintillaEditExtern::removeDebugPoint(int line)
{
    markerDelete(line, StyleSci::Debug);
}

newlsp::ProjectKey ScintillaEditExtern::projectKey() const
{
    return d->proKey;
}

QString ScintillaEditExtern::language() const
{
    return QString::fromStdString(d->proKey.language);
}

void ScintillaEditExtern::debugPointAllDelete()
{
    markerDeleteAll(StyleSci::Debug);
}

void ScintillaEditExtern::jumpToLine(int line)
{
    int lineOffSet = line - 1;
    int displayLines = linesOnScreen();
    setFocus(true);
    gotoPos(lineEndPosition(lineOffSet));
    if (displayLines > 0) {
        int offsetLines = displayLines / 2;
        setFirstVisibleLine(qMax(0, lineOffSet - offsetLines));
    }
    cancel();
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
    d->isSaveText = true;
    file.write(textRange(0, length()));
    emit saved(d->filePath);
    file.close();
}


void ScintillaEditExtern::saveAsText()
{
    QFile file(d->filePath);
    if (!file.open(QFile::ReadWrite | QFile::Text | QFile::Truncate)) {
        ContextDialog::ok("Can't save current: " + file.errorString());
        return;
    }
    d->isSaveText = true;
    file.write(textRange(0, length()));
    emit saved(d->filePath);
    file.close();
}

bool ScintillaEditExtern::isSaveText()
{
    return d->isSaveText;
}

void ScintillaEditExtern::cleanIsSaveText()
{
    d->isSaveText = false;
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
    return ScintillaEdit::findText(SCFIND_NONE, text.toLatin1().data(), start, end);
}

void ScintillaEditExtern::findText(const QString &srcText, bool reverse)
{
    long int currentPos = ScintillaEditExtern::currentPos();
    long int maxPos = length();
    long int startPos = reverse ? (currentPos - srcText.length()) : currentPos;
    long int endPos = reverse ? 0 : maxPos;
    QPair<int, int> position = ScintillaEditExtern::findText(startPos, endPos, srcText.toLatin1().data());
    if (position.first >= 0) {
        ScintillaEditExtern::jumpToRange(position.first, position.second);
    } else {
        if (reverse) {
            if (position.second == 0) {
                findText(srcText, maxPos, 0);
            }
        } else {
            if (position.second == maxPos) {
                findText(srcText, 0, maxPos);
            }
        }
    }
}

void ScintillaEditExtern::findText(const QString &srcText, long int startPos, long int endPos)
{
    QPair<int, int> position = ScintillaEditExtern::findText(startPos, endPos, srcText.toLatin1().data());
    if (position.first >= 0) {
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

void ScintillaEditExtern::setLineBackground(int line, const QColor &color)
{
    int lineOffSet = line - 1;
    markerAdd(lineOffSet, StyleSci::CustomLineBackground);
    markerSetBack(StyleSci::CustomLineBackground, StyleColor::color(color));
    markerSetAlpha(StyleSci::CustomLineBackground, color.alpha());
}

void ScintillaEditExtern::delLineBackground(int line)
{
    int lineOffSet = line - 1;
    markerDelete(lineOffSet, StyleSci::CustomLineBackground);
}

void ScintillaEditExtern::cleanLineBackground()
{
    markerDeleteAll(StyleSci::CustomLineBackground);
}

void ScintillaEditExtern::setAnnotation(int line, const QString &title, const AnnotationInfo &info)
{
    int lineOffset = line - 1;
    if (title.isEmpty()) {
        if (d->lineAnnotations.keys().contains(lineOffset)) {
            auto anns = d->lineAnnotations.value(lineOffset);
            if (anns.contains(info)) {
                return;
            } else {
                anns.insert(0, info);
                d->lineAnnotations[lineOffset] = anns;
            }
        } else {
            d->lineAnnotations[lineOffset] = {info};
        }
    } else {
        if (d->moduleAnnotations.keys().contains(title)) {
            auto lineAnns = d->moduleAnnotations.value(title);
            if(lineAnns.keys().contains(lineOffset)) {
                auto anns = lineAnns.value(lineOffset);
                if (anns.contains(info))
                    return;
                else {
                    anns.insert(0, info);
                    lineAnns[lineOffset] = anns;
                    d->moduleAnnotations[title] = lineAnns;
                }
            } else {
                lineAnns[lineOffset] = {info};
                d->moduleAnnotations[title] = lineAnns;
            }
        } else {
            decltype (d->lineAnnotations) lineAnns;
            lineAnns[lineOffset] = {info};
            d->moduleAnnotations[title] = lineAnns;
        }
    }

    sciUpdateAnnotation();
}

void ScintillaEditExtern::cleanAnnotation(const QString &title)
{
    if (title.isEmpty()) {
        d->lineAnnotations.clear();
    } else {
        d->moduleAnnotations.remove(title);
    }

    sciUpdateAnnotation();
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

void ScintillaEditExtern::sciUpdateAnnotation()
{
    annotationClearAll();

    auto getAnnotationStyles = [=](int roleCode, const QString &srcText) -> QString
    {
        QString styles;
        int styleCode = roleCode - annotationStyleOffset();
        styles.resize(srcText.size(), styleCode);
        return styles;
    };

    auto doSetAnnotation = [=] (const QHash<int, QList<AnnotationInfo>> &lineAnns, const QString &title = "")
    {
        QString annHead = "";
        if (!title.isEmpty()) {
            annHead += "  ";
        }

        auto lines = lineAnns.keys();
        for (auto line : lines) {
            auto anns = lineAnns.value(line);
            QHash<QString, QStringList> roleAnns;
            for (int i = 0; i < AnnotationInfo::Role::count(); i++) {
                QString currRoleDisplay = AnnotationInfo::Role::get()->value(i).display;
                auto itera = anns.begin();
                while (itera != anns.end()) {
                    if (itera->role.display == currRoleDisplay) {
                        QStringList annLines;
                        if (roleAnns.keys().contains(itera->role.display)) {
                            annLines = roleAnns.value(itera->role.display);
                        }
                        annLines.push_front(annHead + "  " + itera->text);
                        roleAnns[itera->role.display] = annLines;
                    }
                    itera ++;
                }
            }
            auto roleAnnsItera = roleAnns.begin();
            while (roleAnnsItera != roleAnns.end()) {
                AnnotationInfo::Role::type_value roleElem;
                for (auto idx = 0; idx < AnnotationInfo::Role::count(); idx ++) {
                    auto roleVal = AnnotationInfo::Role::value(idx);
                    if (roleVal.display == roleAnnsItera.key()) {
                        roleElem = roleVal;
                        break;
                    }
                }

                QString lineAnnsText = annHead + roleAnnsItera.key() + ":\n" + roleAnnsItera.value().join("\n");
                if (!title.isEmpty()) {
                    lineAnnsText = title + ":\n" + lineAnnsText;
                }
                QString lineAnnsTextStyles = getAnnotationStyles(roleElem.code, lineAnnsText);

                QString srcAnnsText = annotationText(line);
                QString srcAnnsTextStyles = annotationStyles(line);
                if (!srcAnnsText.isEmpty()) {
                    srcAnnsText += "\n";
                    srcAnnsTextStyles.resize(srcAnnsText.size(), srcAnnsTextStyles[srcAnnsText.size() -1]);
                }
                QString dstLineAnnsText = srcAnnsText + lineAnnsText;
                QString dstLineAnnsTextStyles = srcAnnsTextStyles + lineAnnsTextStyles;
                annotationSetText(line, dstLineAnnsText.toStdString().c_str());
                annotationSetStyles(line, dstLineAnnsTextStyles.toStdString().c_str());
                roleAnnsItera ++;
            }
        }
    };

    // global
    doSetAnnotation(d->lineAnnotations);

    // moduel
    auto itera = d->moduleAnnotations.begin();
    while (itera != d->moduleAnnotations.end()) {
        doSetAnnotation(d->moduleAnnotations.value(itera.key()), itera.key());
        itera ++;
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
            editor.addadDebugPoint(file(), qint64(line + 1)); //line begin 1 from debug point setting
            editor.addDebugPoint(file(), qint64(line + 1));
        } else {
            markerDelete(line, StyleSci::Debug);
            editor.removedDebugPoint(file(), qint64(line + 1)); //line begin 1 from debug point setting
            editor.removeDebugPoint(file(), qint64(line + 1));
        }
    }
}

void ScintillaEditExtern::focusInEvent(QFocusEvent *event)
{
    focusChanged(true);
    return ScintillaEdit::focusInEvent(event);
}

void ScintillaEditExtern::focusOutEvent(QFocusEvent *event)
{
    focusChanged(false);
    callTipCancel();
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

void ScintillaEditExtern::find(const QString &srcText, int operateType)
{
    switch (operateType) {
    case FindType::Previous:
    {
        searchAnchor();
        findText(srcText, true);
        break;
    }
    case FindType::Next:
    {
        searchAnchor();
        findText(srcText, false);
        break;
    }
    default:
        break;
    }
}

void ScintillaEditExtern::replace(const QString &srcText, const QString &destText, int operateType)
{
    switch (operateType) {
    case RepalceType::Repalce:
    {
        QByteArray byteArray = getSelText();
        if (0 == QString(byteArray).compare(srcText, Qt::CaseInsensitive)) {
            replaceSel(destText.toLatin1().data());
        }

        break;
    }
    case RepalceType::FindAndReplace:
    {
        QByteArray byteArray = getSelText();
        if (0 == QString(byteArray).compare(srcText, Qt::CaseInsensitive)) {
            replaceSel(destText.toLatin1().data());
        }

        searchAnchor();
        findText(srcText, false);
        break;
    }
    case RepalceType::RepalceAll:
    {
        replaceAll(srcText, destText);
        break;
    }
    default:
        break;
    }
}
