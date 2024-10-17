// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputeditwidget.h"
#include "referencepopup.h"
#include "codegeexmanager.h"
#include "services/editor/editorservice.h"

#include <DTextEdit>
#include <DToolButton>
#include <DGuiApplicationHelper>

#include <QKeyEvent>
#include <QPainter>
#include <QStyleOptionFrame>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QAbstractTextDocumentLayout>

DWIDGET_USE_NAMESPACE

static const int minInputEditHeight = 36;
static const int maxInputEditHeight = 236;

static const QString reference_opened_files = "OpendFiles";
static const QString reference_current_file = "CurrentFile";
static const QString reference_select_file = "SelectFile";
static const QString reference_codebase = "CodeBase";

TagTextFormat::TagTextFormat()
    : QTextCharFormat(QTextFormat(QTextFormat::InvalidFormat))
{
    setObjectType(QTextFormat::UserObject + 1);
}

void TagTextFormat::setText(const QString &text)
{
    setProperty(QTextFormat::UserProperty, text);
}

TagTextFormat::TagTextFormat(const QTextFormat &fmt)
    : QTextCharFormat(fmt)
{
}

class TagObjectInterface : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
                         const QTextFormat &format) override
    {
        Q_UNUSED(doc);
        Q_UNUSED(posInDocument);
        const TagTextFormat tagFormat(format);
        const QFontMetricsF fm(tagFormat.font());
        return QSizeF(fm.horizontalAdvance(tagFormat.text()) + 5, fm.height());
    }

    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                    int posInDocument, const QTextFormat &format) override
    {
        Q_UNUSED(doc);
        Q_UNUSED(posInDocument);
        const TagTextFormat tagFormat(format);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        using DTK_GUI_NAMESPACE::DPalette;

        DPalette dp(DTK_GUI_NAMESPACE::DGuiApplicationHelper::instance()->applicationPalette());

        auto color = dp.color(DPalette::LightLively);
        color.setAlpha(26);
        painter->setBrush(color);

        const QFontMetricsF fontMetrics(tagFormat.font());
        QRectF tagRect(rect.x(), rect.y(), rect.width(), fontMetrics.height() + 5);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(tagRect, 5, 5);
        QPen pen(dp.color(DPalette::LightLively));
        pen.setWidth(0);
        painter->setPen(pen);
        painter->drawText(tagRect, Qt::AlignCenter, tagFormat.text());
        painter->restore();
    }
};

QString TagTextFormat::text() const
{
    return property(QTextFormat::UserProperty).toString();
}

class InputEditWidgetPrivate
{
public:
    explicit InputEditWidgetPrivate(InputEditWidget *qq);
    InputEditWidget *q;
    InputEdit *edit { nullptr };
    DToolButton *sendButton { nullptr };
    DToolButton *netWorkBtn { nullptr };
    DToolButton *referenceBtn { nullptr };

    QWidget *buttonBox { nullptr };

    PopupWidget *referencePopup { nullptr };
    QList<ItemInfo> defaultReferenceItems;
    ItemModel model;

    QStringList selectedFiles;
    QMap<QString, QStringList> tagMap;

private:
    void initEdit();
    void initButtonBox();
    void initreferencePopup();
};

InputEditWidgetPrivate::InputEditWidgetPrivate(InputEditWidget *qq)
    : q(qq)
{
    initEdit();
    initButtonBox();
    initreferencePopup();
}

void InputEditWidgetPrivate::initEdit()
{
    edit = new InputEdit(q);
    InputEditWidget::connect(edit, &InputEdit::textChanged, q, [this]() {
        if (edit->toPlainText().isEmpty())
            sendButton->setEnabled(false);
        else
            sendButton->setEnabled(true);

        q->setFixedHeight(edit->height() + buttonBox->height());
        auto cursorPos = edit->textCursor().position();
        if (cursorPos > 0 && edit->document()->characterAt(cursorPos - 1) == "@")
            q->popupReference();
    });
}

void InputEditWidgetPrivate::initButtonBox()
{
    buttonBox = new QWidget(q);
    buttonBox->setFixedHeight(minInputEditHeight);
    auto hLayout = new QHBoxLayout(buttonBox);
    hLayout->setContentsMargins(6, 6, 6, 6);
    hLayout->setAlignment(Qt::AlignRight);
    hLayout->setSpacing(0);

    sendButton = new DToolButton(q);
    sendButton->setFixedSize(24, 24);
    sendButton->setIcon(QIcon::fromTheme("codegeex_send"));
    sendButton->setEnabled(false);

    referenceBtn = new DToolButton(q);
    referenceBtn->setFixedSize(24, 24);
    referenceBtn->setIcon(QIcon::fromTheme("codegeex_files"));
    referenceBtn->setToolTip(InputEditWidget::tr("reference files"));

    netWorkBtn = new DToolButton(q);
    netWorkBtn->setFixedSize(24, 24);
    netWorkBtn->setCheckable(true);
    netWorkBtn->setIcon(QIcon::fromTheme("codegeex_internet"));
    netWorkBtn->setToolTip(InputEditWidget::tr("connect to network"));

    InputEditWidget::connect(sendButton, &DToolButton::clicked, q, &InputEditWidget::messageSended);
    InputEditWidget::connect(referenceBtn, &DToolButton::clicked, q, &InputEditWidget::onReferenceBtnClicked);
    InputEditWidget::connect(netWorkBtn, &DToolButton::clicked, q, &InputEditWidget::onNetWorkBtnClicked);
    hLayout->addWidget(referenceBtn);
    hLayout->addWidget(netWorkBtn);
    hLayout->addWidget(sendButton);
}

void InputEditWidgetPrivate::initreferencePopup()
{
    referencePopup = new PopupWidget(q);
    referencePopup->setWindowFlags(Qt::ToolTip);
    referencePopup->setmodel(&model);

    ItemInfo currentFile;
    currentFile.type = reference_current_file;
    currentFile.displayName = InputEditWidget::tr("Current File");
    ItemInfo selectFile;
    selectFile.type = reference_select_file;
    selectFile.displayName = InputEditWidget::tr("Select File");
    ItemInfo openedFiles;
    openedFiles.type = reference_opened_files;
    openedFiles.displayName = InputEditWidget::tr("Opened Files");
#ifdef SUPPORTMINIFORGE
        ItemInfo codeBase;
        codeBase.type = reference_codebase;
        codeBase.displayName = InputEditWidget::tr("CodeBase");
        defaultReferenceItems = QList { currentFile, selectFile, openedFiles, codeBase };
#else
        defaultReferenceItems = QList { currentFile, selectFile, openedFiles };
#endif
}

InputEdit::InputEdit(QWidget *parent)
    : DTextEdit(parent)
{
    setMinimumHeight(minInputEditHeight);
    setFixedHeight(minInputEditHeight);
    setLineWrapMode(QTextEdit::WidgetWidth);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    document()->documentLayout()->registerHandler(QTextFormat::UserObject + 1, new TagObjectInterface);

    connect(this, &DTextEdit::textChanged, this, &InputEdit::onTextChanged);
}

void InputEdit::onTextChanged()
{
    auto adjustHeight = document()->size().height();
    if (adjustHeight < minInputEditHeight)
        setFixedHeight(minInputEditHeight);
    else if (adjustHeight > maxInputEditHeight)
        setFixedHeight(maxInputEditHeight);
    else
        setFixedHeight(adjustHeight);

    QTextCursor cursor(document());
    QSet<QString> tagList;
    int last_pos = 0;

    // bug: tag will removed when send message. and causes the tag to be reset before it is used.
    // update tag when codegeex is not running
    if (CodeGeeXManager::instance()->checkRunningState(true))
        return;

    cursor.setPosition(0);
    formatList.clear();

    while (!cursor.atEnd()) {
        cursor.setPosition(last_pos + 1);

        if (last_pos == cursor.position())
            break;
        last_pos = cursor.position();

        TagTextFormat format(cursor.charFormat());
        if (format.objectType() == QTextFormat::UserObject + 1) {
            const QString &text = format.text();

            if (!text.isEmpty()) {
                tagList << text;
                formatList << text;

                if (!formats.contains(text)) {
                    formats[text] = format;
                    Q_EMIT tagAdded(text);
                }
            }
        }
    }

    Q_FOREACH (const TagTextFormat &f, formats) {
        if (!tagList.contains(f.text())) {
            formats.remove(f.text());
            Q_EMIT tagRemoved(f.text());
        }
    }
}

bool InputEdit::event(QEvent *e)
{
    if (e->type() == QEvent::Show)
        setFocus();

    return DTextEdit::event(e);
}

void InputEdit::focusOutEvent(QFocusEvent *e)
{
    DTextEdit::focusOutEvent(e);
    emit focusOut();
}

QString InputEdit::toPlainText() const
{
    return DTextEdit::toPlainText().remove(QChar::ObjectReplacementCharacter);
}

void InputEdit::appendTag(const QString &text)
{
    auto currentCursor = textCursor();
    while (currentCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor)) {
        if (currentCursor.selectedText().at(0) == "@")
            break;
    }

    auto selectedText = currentCursor.selectedText();
    if (selectedText.startsWith('@'))
        currentCursor.removeSelectedText();

    auto tagText = "@" + text;

    auto oldFormat = textCursor().charFormat();
    TagTextFormat format;
    format.setText(tagText);
    formats.insert(tagText, format);

    textCursor().insertText(QString(QChar::ObjectReplacementCharacter), format);
    textCursor().insertText(QString(QChar::ObjectReplacementCharacter), oldFormat);   //  to reset textCharFormat
}

void InputEditWidget::onReferenceBtnClicked()
{
    d->edit->append("@");
}

void InputEditWidget::onNetWorkBtnClicked()
{
    CodeGeeXManager::instance()->connectToNetWork(d->netWorkBtn->isChecked());
}

InputEditWidget::InputEditWidget(QWidget *parent)
    : DFrame(parent), d(new InputEditWidgetPrivate(this))
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(d->edit);
    mainLayout->addWidget(d->buttonBox);
    installEventFilter(this);
    d->edit->installEventFilter(this);

    setFixedHeight(d->edit->height() + d->buttonBox->height());
    connect(this, &InputEditWidget::handleKey, d->referencePopup, &PopupWidget::keyPressEvent);
    connect(d->edit, &InputEdit::enterReference, this, &InputEditWidget::popupReference);
    connect(d->referencePopup, &PopupWidget::selectIndex, this, &InputEditWidget::accept);
    connect(d->edit, &InputEdit::focusOut, d->referencePopup, &PopupWidget::hide);
    connect(d->edit, &InputEdit::tagAdded, this, &InputEditWidget::onTagAdded);
    connect(d->edit, &InputEdit::tagRemoved, this, &InputEditWidget::onTagRemoved);
}

bool InputEditWidget::event(QEvent *e)
{
    if (e->type() == QEvent::Paint) {
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing);

        QStyleOptionFrame panel;
        initStyleOption(&panel);
        style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);

        return true;
    }

    return DFrame::event(e);
}

bool InputEditWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->edit) {
        if (event->type() == QEvent::Paint) {
            return true;   // do not show border when get focus
        } else if (event->type() == QEvent::KeyPress) {
            auto keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
            case Qt::Key_Down:
            case Qt::Key_Tab:
            case Qt::Key_Up:
            case Qt::Key_Backtab:
                emit handleKey(keyEvent);
                return true;
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (keyEvent->modifiers() & Qt::AltModifier)
                    d->edit->insertPlainText("\n");
                else if (!d->referencePopup->isVisible())
                    emit pressedEnter();
                else
                    emit handleKey(keyEvent);
                return true;
            case Qt::Key_Backspace:
            case Qt::Key_Space:
                d->referencePopup->hide();
                break;
            default:
                break;
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

InputEdit *InputEditWidget::edit()
{
    return d->edit;
}

void InputEditWidget::popupReference()
{
    d->model.clear();
    d->tagMap.clear();
    d->model.addItems(d->defaultReferenceItems);
    d->referencePopup->show();
    d->referencePopup->selectFirstRow();
}

void InputEditWidget::accept(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    auto row = index.row();
    if (row < 0 || row >= d->model.rowCount())
        return;

    using dpfservice::EditorService;
    EditorService *editorSrv = dpfGetService(EditorService);
    ItemInfo item = d->model.getItems().at(row);

    auto appendTag = [=](const QString &filePath) {
        QFileInfo info(filePath);
        d->selectedFiles.append(filePath);
        auto tag = "file: " + info.dir().dirName() + '/' + info.fileName();
        d->edit->appendTag(tag);
        d->tagMap.insert('@' + tag, { filePath });
    };
    if (item.type == reference_current_file) {
        auto filePath = editorSrv->currentFile();
        if (filePath.isEmpty())
            return;
        appendTag(filePath);
    } else if (item.type == reference_select_file) {
        QString result = QFileDialog::getOpenFileName(this, QAction::tr("Select File"), QDir::homePath());
        if (result.isEmpty())
            return;
        appendTag(result);
    } else if (item.type == reference_opened_files) {
        auto openedFiles = editorSrv->openedFiles();
        if (openedFiles.isEmpty())
            return;
        QList<ItemInfo> items;
        for (auto file : openedFiles) {
            ItemInfo item;
            item.extraInfo = file;
            item.displayName = QFileInfo(file).fileName();
            items.append(item);
        }
        d->model.clear();
        d->model.addItems(items);
        return;
    } else if (item.type == reference_codebase) {
        CodeGeeXManager::instance()->setReferenceCodebase(true);
        d->edit->appendTag(reference_codebase);
    } else if (!item.extraInfo.isEmpty()) {
        appendTag(item.extraInfo);
    }

    d->referencePopup->hide();
    CodeGeeXManager::instance()->setReferenceFiles(d->selectedFiles);
}

// use to restore tag, : remove tag then Ctrl+z
void InputEditWidget::onTagAdded(const QString &text)
{
    if (text.mid(1) == reference_codebase)
        CodeGeeXManager::instance()->setReferenceCodebase(true);
    if (!d->tagMap.contains(text))
        return;
    d->selectedFiles.append(d->tagMap[text]);
    CodeGeeXManager::instance()->setReferenceFiles(d->selectedFiles);
}

void InputEditWidget::onTagRemoved(const QString &text)
{
    if (text.mid(1) == reference_codebase) //remove @
        CodeGeeXManager::instance()->setReferenceCodebase(false);
    if (!d->tagMap.contains(text))
        return;
    for (auto item : d->tagMap[text])
        d->selectedFiles.removeOne(item);
    CodeGeeXManager::instance()->setReferenceFiles(d->selectedFiles);
}

#include "inputeditwidget.moc"
