// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "linechatwidget.h"
#include "inputeditwidget.h"
#include "copilot.h"
#include "diff_match_patch.h"
#include "codegeex/askapi.h"
#include "codegeexmanager.h"

#include "services/editor/editorservice.h"

#include <DToolButton>
#include <DIconButton>
#include <DSuggestButton>
#include <DSpinner>
#include <DLabel>
#include <DPushButton>

#include <QAction>
#include <QRegExp>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QFutureWatcher>
#include <QtConcurrent>

constexpr char VisibleProperty[] { "VisibleProperty" };
constexpr char UrlSSEChat[] = "https://codegeex.cn/prod/code/chatCodeSseV3/chat";

using namespace dpfservice;
DWIDGET_USE_NAMESPACE

class LineChatWidgetPrivate : public QObject
{
public:
    enum State {
        None = 0,
        Original = 1,
        FollowQuestion = 1 << 1,
        FollowSubmit = 1 << 2,
        SubmitStart = 1 << 3,
        SubmitComplete = 1 << 4,
        QuestionStart = 1 << 5,
        QuestionComplete = 1 << 6,
        ReadyAsk = FollowQuestion | FollowSubmit | 1 << 7,

        AllState = 0xFFFFFFFF
    };
    Q_DECLARE_FLAGS(VisibleFlags, State)

    enum ButtonType {
        PushButton,
        ToolButton,
        SuggestButton
    };

    struct ChatInfo
    {
        QList<Diff> diffList;
        QString fileName;
        QString originalText;
        QString tempText;
        Edit::Range originalRange;
        QMultiMap<Operation, Edit::Range> operationRange;

        void clear()
        {
            diffList.clear();
            fileName.clear();
            originalText.clear();
            tempText.clear();
            originalRange = {};
            operationRange.clear();
        }
    };

    explicit LineChatWidgetPrivate(LineChatWidget *qq);

    void initUI();
    void initConnection();

    QAbstractButton *createButton(const QString &name, const QIcon &icon, ButtonType type, int flags);
    void setState(State s);

    void handleTextChanged();
    void handleSubmitEdit();
    void handleQuickQuestion();
    void handleAskFinished(const QString &msgID, const QString &response, const QString &event);
    void handleAccept();
    void handleReject();
    void handleClose();
    void handleStop();

    void defineBackgroundMarker(const QString &fileName);

    void askForCodeGeeX();
    QList<Diff> diffText(const QString &str1, const QString &str2);
    void processGeneratedData(const QString &data);

public:
    LineChatWidget *q;
    EditorService *editSrv { nullptr };

    DLabel *questionLabel { nullptr };
    DLabel *answerLabel { nullptr };
    QWidget *centralWidget { nullptr };

    InputEdit *edit { nullptr };
    DIconButton *closeBtn { nullptr };
    QAbstractButton *escBtn { nullptr };
    QAbstractButton *submitBtn { nullptr };
    QAbstractButton *questionBtn { nullptr };
    QAbstractButton *acceptBtn { nullptr };
    QAbstractButton *rejectBtn { nullptr };
    QAbstractButton *stopBtn { nullptr };
    DSpinner *spinner { nullptr };

    ChatInfo chatInfo;
    State state { None };
    State prevState { None };
    CodeGeeX::AskApi askApi;
    int deleteMarker = -1;
    int insertMarker = -1;
};

LineChatWidgetPrivate::LineChatWidgetPrivate(LineChatWidget *qq)
    : q(qq)
{
    editSrv = dpfGetService(EditorService);
}

void LineChatWidgetPrivate::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    centralWidget = new QWidget(q);
    centralWidget->setFixedWidth(500);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(centralWidget);

    closeBtn = new DIconButton(q);
    closeBtn->setIcon(QIcon::fromTheme("common_close"));
    closeBtn->setIconSize({ 16, 16 });
    closeBtn->setFlat(true);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    questionLabel = new DLabel(q);
    questionLabel->setWordWrap(true);
    questionLabel->setProperty(VisibleProperty, QuestionComplete | SubmitComplete | FollowQuestion | SubmitStart | QuestionStart | FollowQuestion | FollowSubmit);
    answerLabel = new DLabel(q);
    answerLabel->setProperty(VisibleProperty, QuestionComplete | FollowQuestion);
    answerLabel->setWordWrap(true);
    edit = new InputEdit(q);
    edit->setProperty(VisibleProperty, AllState & ~(SubmitStart | QuestionStart));
    edit->installEventFilter(q);
    q->setFocusProxy(edit);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(0, 0, 0, 0);
    spinner = new DSpinner(q);
    spinner->setFixedSize({ 16, 16 });
    spinner->setProperty(VisibleProperty, SubmitStart | QuestionStart);
    escBtn = createButton(LineChatWidget::tr("Esc to close"), {}, PushButton, Original | QuestionComplete);
    submitBtn = createButton(LineChatWidget::tr("Submit Edit"), QIcon::fromTheme("uc_codegeex_submit"), SuggestButton, ReadyAsk);
    questionBtn = createButton(LineChatWidget::tr("quick question"), QIcon::fromTheme("uc_codegeex_quickquestion"), PushButton, ReadyAsk);
    questionBtn->setIconSize({ 42, 16 });
    stopBtn = createButton(LineChatWidget::tr("Stop"), QIcon::fromTheme("uc_codegeex_reject"), PushButton, SubmitStart | QuestionStart);
    stopBtn->setIconSize({ 42, 16 });
    acceptBtn = createButton(LineChatWidget::tr("Accept"), QIcon::fromTheme("uc_codegeex_accept"), SuggestButton, SubmitComplete);
    acceptBtn->setIconSize({ 42, 16 });
    rejectBtn = createButton(LineChatWidget::tr("Reject"), QIcon::fromTheme("uc_codegeex_reject"), PushButton, SubmitComplete);
    rejectBtn->setIconSize({ 42, 16 });

    btnLayout->addWidget(spinner);
    btnLayout->addWidget(escBtn);
    btnLayout->addWidget(submitBtn);
    btnLayout->addWidget(questionBtn);
    btnLayout->addWidget(stopBtn);
    btnLayout->addWidget(acceptBtn);
    btnLayout->addWidget(rejectBtn);
    btnLayout->addStretch(1);

    contentLayout->addWidget(questionLabel);
    contentLayout->addWidget(answerLabel);
    contentLayout->addWidget(edit);
    contentLayout->addLayout(btnLayout);

    mainLayout->addLayout(contentLayout);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignTop | Qt::AlignRight);
    setState(Original);
}

void LineChatWidgetPrivate::initConnection()
{
    connect(edit, &InputEdit::textChanged, this, &LineChatWidgetPrivate::handleTextChanged);
    connect(closeBtn, &QAbstractButton::clicked, this, &LineChatWidgetPrivate::handleReject);
    connect(escBtn, &QAbstractButton::clicked, this, &LineChatWidgetPrivate::handleReject);
    connect(submitBtn, &QAbstractButton::clicked, this, &LineChatWidgetPrivate::handleSubmitEdit);
    connect(questionBtn, &QAbstractButton::clicked, this, &LineChatWidgetPrivate::handleQuickQuestion);
    connect(acceptBtn, &QAbstractButton::clicked, this, &LineChatWidgetPrivate::handleAccept);
    connect(rejectBtn, &QAbstractButton::clicked, this, &LineChatWidgetPrivate::handleReject);
    connect(stopBtn, &QAbstractButton::clicked, this, &LineChatWidgetPrivate::handleStop);

    connect(&askApi, &CodeGeeX::AskApi::response, this, &LineChatWidgetPrivate::handleAskFinished);
}

QAbstractButton *LineChatWidgetPrivate::createButton(const QString &name, const QIcon &icon, ButtonType type, int flags)
{
    QAbstractButton *btn { nullptr };
    switch (type) {
    case ToolButton:
        btn = new DToolButton(q);
        break;
    case SuggestButton:
        btn = new DSuggestButton(q);
        break;
    case PushButton:
    default:
        btn = new DPushButton(q);
        break;
    }

    btn->setProperty(VisibleProperty, flags);
    if (!name.isEmpty())
        btn->setText(name);
    if (!icon.isNull())
        btn->setIcon(icon);

    auto f = btn->font();
    f.setPointSize(12);
    btn->setFont(f);
    return btn;
}

void LineChatWidgetPrivate::setState(State s)
{
    if (state == s)
        return;

    switch (s) {
    case Original:
        edit->setPlaceholderText(LineChatWidget::tr("Ask here by pressing Enter to send your question"));
        break;
    case SubmitStart:
    case QuestionStart:
        spinner->start();
        break;
    case SubmitComplete:
    case QuestionComplete:
        spinner->stop();
        edit->setPlaceholderText(LineChatWidget::tr("Follow-up or new code instructions"));
        break;
    default:
        break;
    }

    prevState = state;
    state = s;
    for (auto child : centralWidget->children()) {
        auto w = qobject_cast<QWidget *>(child);
        if (!w)
            continue;

        auto property = child->property(VisibleProperty);
        if (!property.isValid())
            continue;

        auto flags = static_cast<VisibleFlags>(property.toInt());
        w->setVisible(flags.testFlag(s));
    }
}

void LineChatWidgetPrivate::handleTextChanged()
{
    const auto &text = edit->toPlainText();
    questionLabel->setEnabled(text.isEmpty());
    answerLabel->setEnabled(text.isEmpty());
    if (text.isEmpty()) {
        setState(prevState);
        return;
    }

    if (state == Original || state == ReadyAsk)
        setState(ReadyAsk);
    else if (state == SubmitComplete || state == FollowSubmit)
        setState(FollowSubmit);
    else
        setState(FollowQuestion);
}

void LineChatWidgetPrivate::handleSubmitEdit()
{
    setState(SubmitStart);
    askForCodeGeeX();
}

void LineChatWidgetPrivate::handleQuickQuestion()
{
    setState(QuestionStart);
    askForCodeGeeX();
}

void LineChatWidgetPrivate::handleAskFinished(const QString &msgID, const QString &response, const QString &event)
{
    if (state == QuestionStart || state == QuestionComplete) {
        if (event == "add") {
            auto answer = answerLabel->text();
            answerLabel->setText(answer.append(response));
        }
        setState(QuestionComplete);
    } else if (state == SubmitStart && event == "finish") {
        // Extract the code block in `response`
        QRegularExpression regex(R"(```\w*\n((.*\n)*.*)\n```)");
        QRegularExpressionMatch match = regex.match(response);
        if (!match.hasMatch()) {
            setState(Original);
            return;
        }

        processGeneratedData(match.captured(1));
        setState(SubmitComplete);
    }
}

void LineChatWidgetPrivate::handleAccept()
{
    diff_match_patch dmp;
    auto patchs = dmp.patch_make(chatInfo.diffList);
    auto dstInfo = dmp.patch_apply(patchs, chatInfo.originalText);
    QString replaceText = dstInfo.first;
    if (replaceText.isEmpty())
        replaceText = chatInfo.originalText;
    if (!replaceText.endsWith('\n'))
        replaceText.append('\n');

    int endLineOffset = chatInfo.tempText.count('\n') - chatInfo.originalText.count('\n') - 1;
    Edit::Range replaceRange = chatInfo.originalRange;
    replaceRange.end.line += endLineOffset;
    editSrv->replaceRange(chatInfo.fileName, replaceRange, replaceText);
    handleClose();
}

void LineChatWidgetPrivate::handleReject()
{
    if (!chatInfo.tempText.isEmpty()) {
        QString replaceText = chatInfo.originalText;
        if (!replaceText.endsWith('\n'))
            replaceText.append('\n');

        int endLineOffset = chatInfo.tempText.count('\n') - chatInfo.originalText.count('\n') - 1;
        Edit::Range replaceRange = chatInfo.originalRange;
        replaceRange.end.line += endLineOffset;
        editSrv->replaceRange(chatInfo.fileName, replaceRange, replaceText);
    }
    handleClose();
}

void LineChatWidgetPrivate::handleClose()
{
    editSrv->closeLineWidget();
}

void LineChatWidgetPrivate::handleStop()
{
    setState(prevState);
    edit->setPlainText(questionLabel->text());
    Q_EMIT askApi.stopReceive();
}

void LineChatWidgetPrivate::defineBackgroundMarker(const QString &fileName)
{
    QColor insBgColor(230, 240, 208);
    QColor delBgColor(242, 198, 196);

    insertMarker = editSrv->backgroundMarkerDefine(fileName, insBgColor, insertMarker);
    deleteMarker = editSrv->backgroundMarkerDefine(fileName, delBgColor, deleteMarker);
}

void LineChatWidgetPrivate::askForCodeGeeX()
{
    auto f = questionLabel->font();
    f.setUnderline(state == QuestionStart);
    questionLabel->setFont(f);

    auto question = edit->toPlainText();
    if (prevState == FollowSubmit && state == SubmitStart) {
        auto historyQuestion = questionLabel->text();
        if (!historyQuestion.isEmpty())
            question.prepend(historyQuestion + ", ");
    }

    QSignalBlocker blk(edit);
    edit->clear();
    questionLabel->setEnabled(true);
    answerLabel->setEnabled(true);
    questionLabel->setText(question);
    answerLabel->clear();

    if (!chatInfo.originalText.isEmpty()) {
        if (state == QuestionStart) {
            QString format = "根据提供的内容来回答问题，问题：%1，内容：```%2```";
            question = format.arg(question, chatInfo.originalText);
        } else {
            QString format = "基于这段代码和问题，根据问题生成一段代码用来替换，只生成代码。问题：%1， 代码：```%2```";
            question = format.arg(question, chatInfo.originalText);
        }
    }

    QString machineId = QSysInfo::machineUniqueId();
    askApi.postSSEChat(UrlSSEChat, CodeGeeXManager::instance()->getSessionId(),
                       question, machineId, {}, CodeGeeXManager::instance()->getTalkId());
}

QList<Diff> LineChatWidgetPrivate::diffText(const QString &str1, const QString &str2)
{
    diff_match_patch dmp;
    auto a = dmp.diff_linesToChars(str1, str2);
    auto lineText1 = a[0].toString();
    auto lineText2 = a[1].toString();
    auto lineArray = a[2].toStringList();
    auto diffs = dmp.diff_main(lineText1, lineText2, false);
    dmp.diff_charsToLines(diffs, lineArray);
    dmp.diff_cleanupSemantic(diffs);

    return diffs;
}

void LineChatWidgetPrivate::processGeneratedData(const QString &data)
{
    chatInfo.operationRange.clear();
    chatInfo.diffList = diffText(chatInfo.originalText, data);
    int startLine = chatInfo.originalRange.start.line;
    int endLine = 0;
    QString tempText;
    for (auto diff : chatInfo.diffList) {
        if (!diff.text.endsWith('\n'))
            diff.text.append('\n');

        endLine = startLine + diff.text.count('\n') - 1;
        if (diff.operation != EQUAL) {
            Edit::Range range;
            range.start.line = startLine;
            range.end.line = endLine;
            chatInfo.operationRange.insert(diff.operation, range);
        }

        startLine = endLine + 1;
        tempText.append(diff.text);
    }

    chatInfo.tempText = tempText;
    editSrv->replaceRange(chatInfo.fileName, chatInfo.originalRange, tempText);
    defineBackgroundMarker(chatInfo.fileName);
    auto iter = chatInfo.operationRange.cbegin();
    for (; iter != chatInfo.operationRange.cend(); ++iter) {
        const auto &range = iter.value();
        iter.key() == DELETE ? editSrv->setRangeBackgroundColor(chatInfo.fileName, range.start.line, range.end.line, deleteMarker)
                             : editSrv->setRangeBackgroundColor(chatInfo.fileName, range.start.line, range.end.line, insertMarker);
    }
}

LineChatWidget::LineChatWidget(QWidget *parent)
    : QWidget(parent),
      d(new LineChatWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

LineChatWidget::~LineChatWidget()
{
    delete d;
}

void LineChatWidget::showLineChat()
{
    d->chatInfo.clear();
    d->chatInfo.fileName = d->editSrv->currentFile();
    if (d->chatInfo.fileName.isEmpty())
        return;

    const auto &selRange = d->editSrv->selectionRange(d->chatInfo.fileName);
    auto pos = d->editSrv->cursorPosition();
    auto codeRange = d->editSrv->codeRange(d->chatInfo.fileName, pos);
    if (selRange.start.line == -1) {
        if (codeRange.start.line == -1) {
            d->chatInfo.originalRange.start.line = pos.line;
            d->chatInfo.originalRange.end.line = pos.line;
        } else if (codeRange.start.line == pos.line) {
            d->chatInfo.originalRange.start.line = codeRange.start.line;
            d->chatInfo.originalRange.end.line = codeRange.end.line;
        } else {
            d->chatInfo.originalRange.start.line = pos.line;
            d->chatInfo.originalRange.end.line = codeRange.end.line;
        }
    } else {
        if (selRange.start.line == selRange.end.line
            && selRange.start.line == codeRange.start.line) {
            d->chatInfo.originalRange.start.line = codeRange.start.line;
            d->chatInfo.originalRange.end.line = codeRange.end.line;
        } else {
            d->chatInfo.originalRange.start.line = selRange.start.line;
            d->chatInfo.originalRange.end.line = selRange.end.line;
        }
    }

    d->chatInfo.originalText = d->editSrv->rangeText(d->chatInfo.fileName, d->chatInfo.originalRange);
    d->editSrv->showLineWidget(d->chatInfo.originalRange.start.line, this);
}

void LineChatWidget::showEvent(QShowEvent *e)
{
    d->edit->setFocus();
    d->edit->clear();
    d->setState(LineChatWidgetPrivate::Original);
    QWidget::showEvent(e);
}

void LineChatWidget::keyPressEvent(QKeyEvent *e)
{
    switch (e->modifiers()) {
    case Qt::ControlModifier:
        if (e->key() == Qt::Key_Backspace && d->stopBtn->isVisible()) {
            d->handleStop();
            return;
        }
        break;
    case Qt::NoModifier:
        if (e->key() == Qt::Key_Escape) {
            d->handleReject();
            return;
        }
    default:
        break;
    }
    QWidget::keyPressEvent(e);
}

void LineChatWidget::hideEvent(QHideEvent *e)
{
    if (d->insertMarker != -1)
        d->editSrv->clearAllBackgroundColor(d->chatInfo.fileName, d->insertMarker);
    if (d->deleteMarker != -1)
        d->editSrv->clearAllBackgroundColor(d->chatInfo.fileName, d->deleteMarker);
    QWidget::hideEvent(e);
}

bool LineChatWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == d->edit && e->type() == QEvent::KeyPress) {
        auto ke = static_cast<QKeyEvent *>(e);
        switch (ke->modifiers()) {
        case Qt::ShiftModifier: {
            switch (ke->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (d->questionBtn->isVisible()) {
                    d->handleQuickQuestion();
                    return true;
                }
                break;
            default:
                break;
            }
        }
        case Qt::ControlModifier: {
            switch (ke->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (d->acceptBtn->isVisible()) {
                    d->handleAccept();
                    return true;
                }
                break;
            case Qt::Key_Backspace:
                if (d->rejectBtn->isVisible()) {
                    d->handleReject();
                    return true;
                }
                break;
            default:
                break;
            }
        }
        case Qt::NoModifier: {
            switch (ke->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (d->submitBtn->isVisible()) {
                    d->handleSubmitEdit();
                    return true;
                }
                break;
            default:
                break;
            }
        } break;
        }
    }

    return QWidget::eventFilter(obj, e);
}
