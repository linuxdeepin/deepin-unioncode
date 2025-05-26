// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "giteditor.h"
#include "constants.h"
#include "handler/changetextcursorhandler.h"

#include <DGuiApplicationHelper>

#include <QStyle>
#include <QPainter>
#include <QTextBlock>
#include <QCoreApplication>
#include <QRegularExpression>

DGUI_USE_NAMESPACE

class TextEditExtraArea : public QWidget
{
public:
    explicit TextEditExtraArea(GitEditor *edit)
        : QWidget(edit)
    {
        textEdit = edit;
        setAutoFillBackground(true);
    }

protected:
    QSize sizeHint() const override
    {
        return { textEdit->extraAreaWidth(), 0 };
    }
    void paintEvent(QPaintEvent *event) override
    {
        textEdit->extraAreaPaintEvent(event);
    }
    void changeEvent(QEvent *event) override
    {
        if (event->type() == QEvent::PaletteChange)
            QCoreApplication::sendEvent(textEdit, event);
    }
    void wheelEvent(QWheelEvent *event) override
    {
        QCoreApplication::sendEvent(textEdit->viewport(), event);
    }

private:
    GitEditor *textEdit;
};

struct ExtraAreaPaintEventData;

class GitEditorPrivate : public QObject
{
public:
    explicit GitEditorPrivate(GitEditor *qq);

    void initConnection();

    void updateExtraAreaWidth(int width = 0);
    void updateRequest(const QRect &r, int dy);
    QTextBlock nextVisibleBlock(const QTextBlock &block) const;

    AbstractTextCursorHandler *findTextCursorHandler(const QTextCursor &cursor);
    void paintLineNumbers(QPainter &painter,
                          const ExtraAreaPaintEventData &data,
                          const QRectF &blockBoundingRect) const;

public:
    GitEditor *q;

    QWidget *extraArea { nullptr };
    QRegularExpression changeNumberPattern;
    QList<AbstractTextCursorHandler *> textCursorHandlers;
    QString sourceFile;
};

GitEditorPrivate::GitEditorPrivate(GitEditor *qq)
    : q(qq),
      changeNumberPattern(QRegularExpression(ChangePattern))
{
    extraArea = new TextEditExtraArea(q);
    extraArea->setMouseTracking(true);

    textCursorHandlers.append(new ChangeTextCursorHandler(q));

    initConnection();
}

void GitEditorPrivate::initConnection()
{
    connect(q, &QPlainTextEdit::updateRequest, this, &GitEditorPrivate::updateRequest);
    connect(q, &QPlainTextEdit::modificationChanged, extraArea, QOverload<>::of(&QWidget::update));
    connect(q, &QPlainTextEdit::blockCountChanged, this, std::bind(&GitEditorPrivate::updateExtraAreaWidth, this, std::placeholders::_1));
}

void GitEditorPrivate::updateExtraAreaWidth(int width)
{
    if (width == 0)
        width = q->extraAreaWidth();

    QMargins margins;
    if (q->isLeftToRight())
        margins = QMargins(width, 0, 0, 0);
    else
        margins = QMargins(0, 0, width, 0);
    if (margins != q->viewportMargins())
        q->setViewportMargins(margins);
}

void GitEditorPrivate::updateRequest(const QRect &r, int dy)
{
    if (dy) {
        extraArea->scroll(0, dy);
    } else if (r.width() > 4) {   // wider than cursor width, not just cursor blinking
        extraArea->update(0, r.y(), extraArea->width(), r.height());
    }

    if (r.contains(q->viewport()->rect()))
        updateExtraAreaWidth();
}

QTextBlock GitEditorPrivate::nextVisibleBlock(const QTextBlock &block) const
{
    QTextBlock nextVisibleBlock = block.next();
    if (!nextVisibleBlock.isVisible()) {
        nextVisibleBlock = q->document()->findBlockByLineNumber(nextVisibleBlock.firstLineNumber());
        while (nextVisibleBlock.isValid() && !nextVisibleBlock.isVisible())
            nextVisibleBlock = nextVisibleBlock.next();
    }

    return nextVisibleBlock;
}

AbstractTextCursorHandler *GitEditorPrivate::findTextCursorHandler(const QTextCursor &cursor)
{
    for (AbstractTextCursorHandler *handler : std::as_const(textCursorHandlers)) {
        if (handler->findContentsUnderCursor(cursor))
            return handler;
    }

    return nullptr;
}

struct ExtraAreaPaintEventData
{
    ExtraAreaPaintEventData(const GitEditor *editor, GitEditorPrivate *d)
        : doc(editor->document()),
          documentLayout(qobject_cast<QPlainTextDocumentLayout *>(doc->documentLayout())),
          selectionStart(editor->textCursor().selectionStart()),
          selectionEnd(editor->textCursor().selectionEnd()),
          fontMetrics(d->extraArea->font()),
          lineSpacing(fontMetrics.lineSpacing()),
          extraAreaWidth(d->extraArea->width()),
          palette(d->extraArea->palette())
    {
        auto f = d->extraArea->font();
        currentLineNumberFormat.setFont(f);
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            currentLineNumberFormat.setForeground(QBrush("#0081FF"));
        else
            currentLineNumberFormat.setForeground(QBrush(Qt::white));

        palette.setCurrentColorGroup(QPalette::Active);
    }
    QTextBlock block;
    const QTextDocument *doc;
    const QPlainTextDocumentLayout *documentLayout;
    const int selectionStart;
    const int selectionEnd;
    const QFontMetrics fontMetrics;
    int lineSpacing;
    const int extraAreaWidth;
    QPalette palette;
    QTextCharFormat currentLineNumberFormat;
};

void GitEditorPrivate::paintLineNumbers(QPainter &painter,
                                        const ExtraAreaPaintEventData &data,
                                        const QRectF &blockBoundingRect) const
{
    const QString &number = q->lineNumber(data.block.blockNumber());
    const bool selected = ((data.selectionStart < data.block.position() + data.block.length()
                            && data.selectionEnd > data.block.position())
                           || (data.selectionStart == data.selectionEnd && data.selectionEnd == data.block.position()));
    if (selected) {
        painter.save();
        QFont f = painter.font();
        f.setBold(data.currentLineNumberFormat.font().bold());
        f.setItalic(data.currentLineNumberFormat.font().italic());
        painter.setFont(f);
        painter.setPen(data.currentLineNumberFormat.foreground().color());
        if (data.currentLineNumberFormat.background() != Qt::NoBrush) {
            painter.fillRect(QRectF(0, blockBoundingRect.top(),
                                    data.extraAreaWidth, blockBoundingRect.height()),
                             data.currentLineNumberFormat.background().color());
        }
    }
    painter.drawText(QRectF(0, blockBoundingRect.top(),
                            data.extraAreaWidth - 4, blockBoundingRect.height()),
                     Qt::AlignRight,
                     number);
    if (selected)
        painter.restore();
}

GitEditor::GitEditor(QWidget *parent)
    : QPlainTextEdit(parent),
      d(new GitEditorPrivate(this))
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setLayoutDirection(Qt::LeftToRight);
    viewport()->setMouseTracking(true);
    setFrameStyle(QFrame::NoFrame);

    auto f = font();
    f.setFamily("Noto Mono");
    setFont(f);
}

GitEditor::~GitEditor()
{
    delete d;
}

void GitEditor::setSourceFile(const QString &sf)
{
    d->sourceFile = sf;
}

QString GitEditor::sourceFile() const
{
    return d->sourceFile;
}

int GitEditor::extraAreaWidth() const
{
    int space = 6;
    QFont fnt = d->extraArea->font();
    const QFontMetrics linefm(fnt);

    space += linefm.horizontalAdvance(QLatin1Char('9')) * lineNumberDigits();
    if (viewportMargins() != QMargins { isLeftToRight() ? space : 0, 0, isLeftToRight() ? 0 : space, 0 })
        d->updateExtraAreaWidth(space);

    return space;
}

void GitEditor::extraAreaPaintEvent(QPaintEvent *e)
{
    ExtraAreaPaintEventData data(this, d);
    QPainter painter(d->extraArea);

    painter.fillRect(e->rect(), data.palette.color(QPalette::Window));

    data.block = firstVisibleBlock();
    QPointF offset = contentOffset();
    QRectF boundingRect = blockBoundingRect(data.block).translated(offset);
    while (data.block.isValid() && boundingRect.top() <= e->rect().bottom()) {
        if (boundingRect.bottom() >= e->rect().top()) {
            painter.setPen(data.palette.color(QPalette::Dark));

            d->paintLineNumbers(painter, data, boundingRect);
        }

        offset.ry() += boundingRect.height();
        data.block = d->nextVisibleBlock(data.block);
        boundingRect = blockBoundingRect(data.block).translated(offset);
    }
}

QString GitEditor::changeUnderCursor(const QTextCursor &c)
{
    QTextCursor cursor = c;
    // Any number is regarded as change number.
    cursor.select(QTextCursor::WordUnderCursor);
    if (!cursor.hasSelection())
        return {};
    const QString change = cursor.selectedText();
    if (d->changeNumberPattern.match(change).hasMatch())
        return change;
    return {};
}

QString GitEditor::lineNumber(int blockNumber) const
{
    return QString::number(blockNumber + 1);
}

int GitEditor::lineNumberDigits() const
{
    int digits = 2;
    int max = qMax(1, blockCount());
    while (max >= 100) {
        max /= 10;
        ++digits;
    }
    return digits;
}

void GitEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    QRect cr = rect();
    d->extraArea->setGeometry(QStyle::visualRect(layoutDirection(), cr,
                                                 QRect(cr.left() + frameWidth(), cr.top() + frameWidth(),
                                                       extraAreaWidth(), cr.height() - 2 * frameWidth())));
}

void GitEditor::mouseMoveEvent(QMouseEvent *e)
{
    if (e->button()) {
        QPlainTextEdit::mouseMoveEvent(e);
        return;
    }

    bool overrideCursor = false;
    Qt::CursorShape cursorShape;
    const QTextCursor cursor = cursorForPosition(e->pos());

    AbstractTextCursorHandler *handler = d->findTextCursorHandler(cursor);
    if (handler != nullptr) {
        handler->highlightCurrentContents();
        overrideCursor = true;
        cursorShape = Qt::PointingHandCursor;
    } else {
        setExtraSelections(QList<QTextEdit::ExtraSelection>());
        overrideCursor = true;
        cursorShape = Qt::IBeamCursor;
    }

    QPlainTextEdit::mouseMoveEvent(e);
    if (overrideCursor)
        viewport()->setCursor(cursorShape);
}

void GitEditor::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && !(e->modifiers() & Qt::ShiftModifier)) {
        const QTextCursor cursor = cursorForPosition(e->pos());
        AbstractTextCursorHandler *handler = d->findTextCursorHandler(cursor);
        if (handler != nullptr) {
            handler->handleCurrentContents();
            e->accept();
            return;
        }
    }

    QPlainTextEdit::mouseReleaseEvent(e);
}
