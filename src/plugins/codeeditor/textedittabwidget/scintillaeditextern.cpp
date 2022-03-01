#include "scintillaeditextern.h"
#include "style/stylekeeper.h"
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
    bool isCtrlKeyPressed;
    bool isLeave;
    Scintilla::Position hoverPos = -1;
    QTimer hoverTimer;
    QString filePath;
    QString rootPath;
    QString language;
    Scintilla::Position editInsertPostion = -1;
    int editInsertCount = 0;
};

ScintillaEditExtern::ScintillaEditExtern(QWidget *parent)
    : ScintillaEdit (parent)
    , d(new ScintillaEditExternPrivate)
{

}

ScintillaEditExtern::~ScintillaEditExtern()
{
    if (d) {
        delete d;
    }
}

void ScintillaEditExtern::setFile(const QString &path)
{
    if (d->filePath == path) {
        return;
    } else {
        d->filePath = path;
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

    using namespace support_file;
    d->language = Language::id(d->filePath);
    StyleSet set = StyleKeeper::create(d->language);

    if (!set.sci) {
        qCritical() << "Failed, can't create sci style from ScintillaEdit!";
        // abort();
    } else { // sci default;
        set.sci->setStyle(*this);
        set.sci->setLexer(*this);
        set.sci->setKeyWords(*this);
        set.sci->setMargin(*this);
    }

    if (!set.lsp) {
        qCritical() << "Failed, can't create lsp style from ScintillaEdit!";
    } else { // lsp default;
        set.lsp->appendEdit(this);
    }

    setMouseDwellTime(0);
    QObject::connect(this, &ScintillaEditExtern::marginClicked, this, &ScintillaEditExtern::sciMarginClicked, Qt::UniqueConnection);
    QObject::connect(this, &ScintillaEditExtern::modified, this, &ScintillaEditExtern::sciModified, Qt::UniqueConnection);
    QObject::connect(this, &ScintillaEditExtern::dwellStart, this, &ScintillaEditExtern::sciDwellStart, Qt::UniqueConnection);
    QObject::connect(this, &ScintillaEditExtern::dwellEnd, this, &ScintillaEditExtern::sciDwellEnd, Qt::UniqueConnection);

    QObject::connect(&d->hoverTimer, &QTimer::timeout, &d->hoverTimer, [=](){
        this->hovered(d->hoverPos);
        d->hoverTimer.stop();
    }, Qt::UniqueConnection);
}

QString ScintillaEditExtern::file() const
{
    return d->filePath;
}

QString ScintillaEditExtern::langueage() const
{
    return d->language;
}

void ScintillaEditExtern::setRootPath(const QString &path)
{
    d->rootPath = path;
}

QString ScintillaEditExtern::rootPath()
{
    return d->rootPath;
}

void ScintillaEditExtern::debugPointAllDelete()
{
    markerDeleteAll(StyleSci::Debug);
}

void ScintillaEditExtern::jumpToLine(int line)
{
    int offSizeLine = line - 1;
    int displayLines = linesOnScreen();
    if (displayLines > 0) {
        int offsetLines = displayLines / 2;
        if (offSizeLine > offsetLines) {
            setFirstVisibleLine(offSizeLine - offsetLines);
        }
    }
}

void ScintillaEditExtern::runningToLine(int line)
{
    int offSizeLine = line - 1;

    markerDeleteAll(StyleSci::Running);
    markerDeleteAll(StyleSci::RunningLineBackground);

    markerAdd(offSizeLine,StyleSci::Running);
    markerAdd(offSizeLine, StyleSci::RunningLineBackground);
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
        completed(position);
    }

    if (bool(type & Scintilla::ModificationFlags::DeleteText)) {
        textDeleted(position, length, linesAdded, text, line);
        autoCCancel();
        completeCleaned();
    }
}

void ScintillaEditExtern::sciDwellStart(int x, int y)
{
    if (d->hoverPos == -1) {
//        if (d->hoverTimer.isActive()) {
//            d->hoverTimer.stop();
//        }
        d->hoverTimer.start(500); //如果间隔较小，导致收发管道溢出最终程序崩溃
        d->hoverPos = positionFromPoint(x, y);
        return;
    }
}

void ScintillaEditExtern::sciDwellEnd(int x, int y)
{
    if (d->hoverPos != -1) {
        if (d->hoverTimer.isActive()) {
            d->hoverTimer.stop();
        }
        emit hoverCleaned(d->hoverPos);
        d->hoverPos = -1;
        return;
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

    StyleSet set = StyleKeeper::create(d->language);
    if (!set.sci) {
        return;
    }

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
