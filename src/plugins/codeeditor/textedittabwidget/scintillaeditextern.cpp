#include "scintillaeditextern.h"
#include "style/stylekeeper.h"
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
    bool isCtrlKeyPressed;
    bool isLeave;
    Scintilla::Position hoverPos = -1;
    QTimer hoverTimer;
    QTimer definitionHoverTimer;
    QString filePath;
    QString projectWorkspace;
    QString projectLanguage;
    QString language;
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
        auto client = lsp::ClientManager::instance()->get({d->projectWorkspace, d->projectLanguage});
        if (!client) {
            qCritical() << "Failed, client is nullptr"
                        << "project workspace: " << d->projectWorkspace
                        << "project language: " << d->projectLanguage;
        }
        set.lsp->setClient(client);
        set.lsp->appendEdit(this);
    }

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

QString ScintillaEditExtern::file() const
{
    return d->filePath;
}

void ScintillaEditExtern::setHeadInfo(const QString &proWorkspace, const QString &proLanguage)
{
    d->projectWorkspace = proWorkspace;
    d->projectLanguage = proLanguage;
}

QString ScintillaEditExtern::proWorkspace() const
{
    return d->projectWorkspace;
}

QString ScintillaEditExtern::proLanguage() const
{
    return d->projectLanguage;
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
        //        bool isKeyCtrl = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
        //        if (isKeyCtrl) {
        if (d->definitionHoverTimer.isActive()) {
            d->definitionHoverTimer.stop();
        }
        emit definitionHoverCleaned(d->hoverPos);
        //        } else {
        if (d->hoverTimer.isActive()) {
            d->hoverTimer.stop();
        }
        emit hoverCleaned(d->hoverPos);
        //        }
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
