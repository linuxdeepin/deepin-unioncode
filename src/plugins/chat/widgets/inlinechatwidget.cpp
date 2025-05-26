// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inlinechatwidget.h"
#include "inputeditwidget.h"
#include "diff_match_patch.h"
#include "chatmanager.h"

#include "services/editor/editorservice.h"
#include "services/project/projectservice.h"

#include <DToolButton>
#include <DIconButton>
#include <DSuggestButton>
#include <DSpinner>
#include <DLabel>
#include <DPushButton>
#include <DGuiApplicationHelper>

#include <QAction>
#include <QRegularExpression>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QFutureWatcher>
#include <QtConcurrent>

constexpr char kVisibleProperty[] { "VisibleProperty" };

static QString systemenPrompt = "你会为用户回答关于编程、代码、计算机方面的任何问题，并提供格式规范、可以执行、准确安全的代码，并在必要时提供详细的解释。任务：根据用户的Command作答。\n"
"\nYou are working on a task with User in a file and User send a message to you. `【cursor】` indicate the current position of your cursor, delete it after modification.\n\nYour Workflow:\n"
"Step 1: You should working on the command step-by-step:\n1. Does the command is about to write program or comments in programming task? Write out the type of the command: (Choose: Chat / Programming).\n- **Chat command**: When your aim to reply through explanations, reminders, or requests for additional information. \n- **Programming command**: The user command is clear and requires you to write program or comments. \n"
"Step 2: If it's a chat command, provide a thoughtful and clear response {language} directly.\n"
"Step 3: If it requires programming, you should complete the Task according to the given file. \na. Understand the message in order to tackle it correctly: is the task about coding or debugging?\n- For coding, add new code within the task to execute the user's instructions correctly. \n- For debugging, improve the code to fix the bug, write the reasons for the changes within your code as comments.\nb. You should complete the task according to user request and comment in Chinese within your code to indicate changes. \nc. You should place only the finished task in a single block as output without explain!!!\n\nReply in the template below:\nCommand Type: (Chat / Programming)\nResponse: (Your response / Finished code in one block)\n"
"User`s question: %1\n"
"Selected code: %2\n"
"Related context about this question(Do not directly quote when answering):<context>%3</context>\n"
"Command:%4";

using namespace Chat;
using namespace dpfservice;
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class InlineChatWidgetPrivate : public QObject
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
        QString destText;
        QString tempText;
        Edit::Range originalRange;
        QMultiMap<Operation, Edit::Range> operationRange;

        void clear()
        {
            diffList.clear();
            fileName.clear();
            originalText.clear();
            destText.clear();
            tempText.clear();
            originalRange = {};
            operationRange.clear();
        }
    };

    struct CodeInfo
    {
        bool isSelectEmpty { false };
        int curosrLine;
    };

    explicit InlineChatWidgetPrivate(InlineChatWidget *qq);
    ~InlineChatWidgetPrivate();

    void initUI();
    void initConnection();

    QAbstractButton *createButton(const QString &name, ButtonType type, int flags);
    void setState(State s);

    void handleTextChanged();
    void handleSubmitEdit();
    void handleQuickQuestion();
    void handleAskFinished(const QString &response);

    void handleAccept();
    void handleReject();
    void handleClose();
    void handleStop();
    void handleCreatePromptFinished();

    void defineBackgroundMarker(const QString &fileName);
    bool askForChat();
    QList<Diff> diffText(const QString &str1, const QString &str2);
    void processGeneratedData(const QString &data);
    void updateButtonIcon();
    QString createPrompt(const QString &question, bool useChunk);
    Edit::Range calculateTextRange(const QString &fileName, const Edit::Position &pos);
    QString addLineNumber(const QString &content, int start);
    QString createFormatCode(const QString &file, const QString &code, const Edit::Range &range);

public:
    InlineChatWidget *q;
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

    QList<QFutureWatcher<QString> *> futureWatcherList;
    ChatInfo chatInfo;
    CodeInfo codeInfo;
    State state { None };
    State prevState { None };
    AbstractLLM *inlineChatLLM { nullptr };
    int deleteMarker { -1 };
    int insertMarker { -1 };
    int selectionMarker { -1 };
    bool isPreinputStatus { false };
};

InlineChatWidgetPrivate::InlineChatWidgetPrivate(InlineChatWidget *qq)
    : q(qq)
{
    editSrv = dpfGetService(EditorService);
}

InlineChatWidgetPrivate::~InlineChatWidgetPrivate()
{
    // Cancel all pending futures.
    foreach (auto watcher, futureWatcherList) {
        if (!watcher->isFinished())
            watcher->cancel();
    }

    // wait for futures to finish
    foreach (auto watcher, futureWatcherList) {
        if (!watcher->isFinished())
            watcher->waitForFinished();
        delete watcher;
    }
}

void InlineChatWidgetPrivate::initUI()
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
    questionLabel->setProperty(kVisibleProperty, QuestionComplete | SubmitComplete | FollowQuestion | SubmitStart | QuestionStart | FollowQuestion | FollowSubmit);
    answerLabel = new DLabel(q);
    answerLabel->setProperty(kVisibleProperty, QuestionComplete | FollowQuestion);
    answerLabel->setWordWrap(true);
    edit = new InputEdit(q);
    edit->setProperty(kVisibleProperty, AllState & ~(SubmitStart | QuestionStart));
    edit->installEventFilter(q);
    q->setFocusProxy(edit);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(0, 0, 0, 0);
    spinner = new DSpinner(q);
    spinner->setFixedSize({ 12, 12 });
    spinner->setProperty(kVisibleProperty, SubmitStart | QuestionStart);
    escBtn = createButton(InlineChatWidget::tr("Esc to close"), PushButton, Original | QuestionComplete);
    submitBtn = createButton(InlineChatWidget::tr("Submit Edit"), SuggestButton, ReadyAsk);
    questionBtn = createButton(InlineChatWidget::tr("quick question"), PushButton, ReadyAsk);
    questionBtn->setIconSize({ 24, 12 });
    stopBtn = createButton(InlineChatWidget::tr("Stop"), PushButton, SubmitStart | QuestionStart);
    stopBtn->setIconSize({ 36, 12 });
    acceptBtn = createButton(InlineChatWidget::tr("Accept"), SuggestButton, SubmitComplete);
    acceptBtn->setIconSize({ 36, 12 });
    rejectBtn = createButton(InlineChatWidget::tr("Reject"), PushButton, SubmitComplete);
    rejectBtn->setIconSize({ 36, 12 });

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

void InlineChatWidgetPrivate::initConnection()
{
    connect(edit, &InputEdit::textChanged, this, &InlineChatWidgetPrivate::handleTextChanged);
    connect(closeBtn, &QAbstractButton::clicked, this, &InlineChatWidgetPrivate::handleClose);
    connect(escBtn, &QAbstractButton::clicked, this, &InlineChatWidgetPrivate::handleClose);
    connect(submitBtn, &QAbstractButton::clicked, this, &InlineChatWidgetPrivate::handleSubmitEdit);
    connect(questionBtn, &QAbstractButton::clicked, this, &InlineChatWidgetPrivate::handleQuickQuestion);
    connect(acceptBtn, &QAbstractButton::clicked, this, &InlineChatWidgetPrivate::handleAccept);
    connect(rejectBtn, &QAbstractButton::clicked, this, &InlineChatWidgetPrivate::handleReject);
    connect(stopBtn, &QAbstractButton::clicked, this, &InlineChatWidgetPrivate::handleStop);
}

QAbstractButton *InlineChatWidgetPrivate::createButton(const QString &name, ButtonType type, int flags)
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

    btn->setFixedHeight(24);
    btn->setProperty(kVisibleProperty, flags);
    if (!name.isEmpty())
        btn->setText(name);

    auto f = btn->font();
    f.setPixelSize(12);
    btn->setFont(f);
    return btn;
}

void InlineChatWidgetPrivate::setState(State s)
{
    if (state == s)
        return;

    q->setFocus();
    switch (s) {
    case Original:
        edit->setPlaceholderText(InlineChatWidget::tr("Ask a question here, press Enter to send"));
        break;
    case SubmitStart:
    case QuestionStart:
        spinner->start();
        break;
    case SubmitComplete:
    case QuestionComplete:
        spinner->stop();
        edit->setPlaceholderText(InlineChatWidget::tr("Continue to ask a question, press Enter to send"));
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

        auto property = child->property(kVisibleProperty);
        if (!property.isValid())
            continue;

        auto flags = static_cast<VisibleFlags>(property.toInt());
        w->setVisible(flags.testFlag(s));
    }
}

void InlineChatWidgetPrivate::handleTextChanged()
{
    // Preinput status is not a real user input
    if (isPreinputStatus) {
        isPreinputStatus = false;
        return;
    }

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

void InlineChatWidgetPrivate::handleSubmitEdit()
{
    if (state == FollowSubmit) {
        handleReject();
        setState(FollowSubmit);
    }

    setState(SubmitStart);

    if (!askForChat()) {
        qWarning() << "Failed to ask";
        setState(Original);
    }
}

void InlineChatWidgetPrivate::handleQuickQuestion()
{
    if (state == FollowSubmit) {
        handleReject();
        setState(FollowSubmit);
    }

    setState(QuestionStart);
    if (!askForChat()) {
        qWarning() << "Failed to ask";
        setState(Original);
    }
}

void InlineChatWidgetPrivate::handleAskFinished(const QString &response)
{
    if (state == QuestionStart) {
        auto answer = answerLabel->text();
        answerLabel->setText(answer.append(response));
        setState(QuestionComplete);
    } else {
        // Extract the code block in `response`
        QString codePart;
        QRegularExpression regex(R"(```.*\n((.*\n)*?.*)\n```)");
        QRegularExpressionMatch match = regex.match(response);
        if (match.hasMatch())
            codePart = match.captured(1);

        if (codePart.isEmpty()) {
            setState(Original);
            return;
        }

        QStringList lines = codePart.split('\n');
        int i = 0;
        // Remove needless datas
        if (codeInfo.isSelectEmpty) {
            while (i < lines.size()) {
                QRegularExpression rx("(\\d+)");
                if (!rx.match(lines[i]).hasMatch()) {
                    ++i;
                    continue;
                }

                // int number = rx.captured(1).toInt();
                int number = rx.match(lines[1]).captured(1).toInt();
                if (number < codeInfo.curosrLine) {
                    lines.removeAt(i);
                    continue;
                }
                break;
            }
        }

        // Remove numbers at the beginning of each line
        for (int i = 0; i < lines.size(); ++i) {
            lines[i] = lines[i].remove(QRegularExpression("^\\d+\\s{2}"));
        }
        codePart = lines.join('\n');
        codePart.remove("【cursor】");

        processGeneratedData(codePart);
        setState(SubmitComplete);
    }
}

void InlineChatWidgetPrivate::handleAccept()
{
    QString replaceText;
    try {
        diff_match_patch dmp;
        auto patchs = dmp.patch_make(chatInfo.diffList);
        auto dstInfo = dmp.patch_apply(patchs, chatInfo.originalText);
        replaceText = dstInfo.first;
    } catch (...) {
        if (chatInfo.originalText.isEmpty())
            replaceText = chatInfo.destText;
        else
            replaceText = chatInfo.originalText;
    }

    if (replaceText.isEmpty())
        replaceText = chatInfo.originalText;
    if (!replaceText.endsWith('\n'))
        replaceText.append('\n');

    int endLineOffset = chatInfo.tempText.count('\n') - chatInfo.originalText.count('\n') - 1;
    Edit::Range replaceRange = chatInfo.originalRange;
    replaceRange.end.line += endLineOffset;
    editSrv->replaceRange(chatInfo.fileName, replaceRange, replaceText);
    chatInfo.tempText.clear();
    handleClose();
}

void InlineChatWidgetPrivate::handleReject()
{
    if (!chatInfo.tempText.isEmpty()) {
        QString replaceText = chatInfo.originalText;
        if (!replaceText.endsWith('\n'))
            replaceText.append('\n');

        int endLineOffset = chatInfo.tempText.count('\n') - chatInfo.originalText.count('\n') - 1;
        chatInfo.tempText.clear();
        Edit::Range replaceRange = chatInfo.originalRange;
        replaceRange.end.line += endLineOffset;
        editSrv->replaceRange(chatInfo.fileName, replaceRange, replaceText);
        editSrv->setRangeBackgroundColor(chatInfo.fileName, chatInfo.originalRange.start.line,
                                         chatInfo.originalRange.end.line, selectionMarker);
    }

    if (insertMarker != -1)
        editSrv->clearAllBackgroundColor(chatInfo.fileName, insertMarker);
    if (deleteMarker != -1)
        editSrv->clearAllBackgroundColor(chatInfo.fileName, deleteMarker);
    setState(Original);
    q->setFocus();
}

void InlineChatWidgetPrivate::handleClose()
{
    editSrv->closeLineWidget();
}

void InlineChatWidgetPrivate::handleStop()
{
    setState(Original);
    edit->setPlainText(questionLabel->text());
    edit->moveCursor(QTextCursor::End);
    questionLabel->clear();
    foreach (auto watcher, futureWatcherList) {
        if (!watcher->isFinished())
            watcher->cancel();
    }
    inlineChatLLM->cancel();
}

void InlineChatWidgetPrivate::handleCreatePromptFinished()
{
    auto watcher = static_cast<QFutureWatcher<QString> *>(sender());
    if (!watcher->isCanceled()) {
        const auto &prompt = watcher->result();
        inlineChatLLM->setStream(false);
        inlineChatLLM->request(prompt, [=](const QString &data, AbstractLLM::ResponseState state){
            if (state == AbstractLLM::Receiving || (state == AbstractLLM::Success && !data.isEmpty()))
                handleAskFinished(data);
            else if (state == AbstractLLM::Failed)
                setState(Original);
        });
    }

    futureWatcherList.removeAll(watcher);
    watcher->deleteLater();
}

void InlineChatWidgetPrivate::defineBackgroundMarker(const QString &fileName)
{
    QColor insBgColor(230, 240, 208);
    QColor delBgColor(242, 198, 196);
    QColor selBgColor(227, 227, 227);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        insBgColor.setRgb(30, 78, 42);
        delBgColor.setRgb(87, 32, 49);
        selBgColor.setRgb(49, 53, 59);
    }

    insertMarker = editSrv->backgroundMarkerDefine(fileName, insBgColor, insertMarker);
    deleteMarker = editSrv->backgroundMarkerDefine(fileName, delBgColor, deleteMarker);
    selectionMarker = editSrv->backgroundMarkerDefine(fileName, selBgColor, selectionMarker);
}

bool InlineChatWidgetPrivate::askForChat()
{
    chatInfo.originalRange = editSrv->getBackgroundRange(chatInfo.fileName, selectionMarker);
    if (chatInfo.originalRange.start.line == -1 || chatInfo.originalRange.end.line == -1)
        return false;
    chatInfo.originalText = editSrv->rangeText(chatInfo.fileName, chatInfo.originalRange);

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
    edit->onTextChanged();
    questionLabel->setEnabled(true);
    answerLabel->setEnabled(true);
    questionLabel->setText(question);
    answerLabel->clear();

    auto *futureWatcher = new QFutureWatcher<QString>();
    futureWatcher->setFuture(QtConcurrent::run([this, question](){ return this->createPrompt(question, false); }));
    connect(futureWatcher, &QFutureWatcher<QString>::finished, this, &InlineChatWidgetPrivate::handleCreatePromptFinished);
    futureWatcherList << futureWatcher;
    return true;
}

QList<Diff> InlineChatWidgetPrivate::diffText(const QString &str1, const QString &str2)
{
    QList<Diff> diffs;
    try {
        diff_match_patch dmp;
        auto a = dmp.diff_linesToChars(str1, str2);
        auto lineText1 = a[0].toString();
        auto lineText2 = a[1].toString();
        auto lineArray = a[2].toStringList();
        diffs = dmp.diff_main(lineText1, lineText2, false);
        dmp.diff_charsToLines(diffs, lineArray);
    } catch (...) {
    }

    return diffs;
}

void InlineChatWidgetPrivate::processGeneratedData(const QString &data)
{
    editSrv->clearAllBackgroundColor(chatInfo.fileName, selectionMarker);
    chatInfo.operationRange.clear();
    chatInfo.diffList.clear();
    chatInfo.destText = data;
    if (chatInfo.originalText.isEmpty())
        chatInfo.diffList << Diff { INSERT, data };
    else
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
    auto iter = chatInfo.operationRange.cbegin();
    for (; iter != chatInfo.operationRange.cend(); ++iter) {
        const auto &range = iter.value();
        iter.key() == DELETE ? editSrv->setRangeBackgroundColor(chatInfo.fileName, range.start.line, range.end.line, deleteMarker)
                             : editSrv->setRangeBackgroundColor(chatInfo.fileName, range.start.line, range.end.line, insertMarker);
    }
}

void InlineChatWidgetPrivate::updateButtonIcon()
{
    submitBtn->setIcon(q->hasFocus() ? QIcon::fromTheme("uc_chat_submit") : QIcon());
    questionBtn->setIcon(q->hasFocus() ? QIcon::fromTheme("uc_chat_quickquestion") : QIcon());
    stopBtn->setIcon(q->hasFocus() ? QIcon::fromTheme("uc_chat_reject") : QIcon());
    acceptBtn->setIcon(q->hasFocus() ? QIcon::fromTheme("uc_chat_accept") : QIcon());
    rejectBtn->setIcon(q->hasFocus() ? QIcon::fromTheme("uc_chat_reject") : QIcon());
}

QString InlineChatWidgetPrivate::createPrompt(const QString &question, bool useChunk)
{
    QString prompt = systemenPrompt;

    QStringList context = {""};
    auto code = createFormatCode(chatInfo.fileName, chatInfo.originalText, chatInfo.originalRange);
    auto fileText = editSrv->fileText(chatInfo.fileName);
    if (useChunk) {
        QString workspace = chatInfo.fileName;
        ProjectService *prjSrv = dpfGetService(ProjectService);
        const auto &allPrjInfo = prjSrv->getAllProjectInfo();
        for (const auto &info : allPrjInfo) {
            if (chatInfo.fileName.startsWith(info.workspaceFolder())) {
                workspace = info.workspaceFolder();
                break;
            }
        }

        auto result = ChatManager::instance()->query(workspace, code, 5);
        QJsonArray chunks = result["Chunks"].toArray();
        context << "<context>";
        for (auto chunk : chunks) {
            context << chunk.toObject()["fileName"].toString();
            context << chunk.toObject()["content"].toString();
        }
        context << "</context>";
    }
    QString command = state == QuestionStart ? "Chat" : "Programing";

    return prompt.arg(question, code, context.join('\n'), command);
}

Edit::Range InlineChatWidgetPrivate::calculateTextRange(const QString &fileName, const Edit::Position &pos)
{
    Edit::Range textRange;
    const auto &selRange = editSrv->selectionRange(fileName);
    auto codeRange = editSrv->codeRange(fileName, pos);
    if (selRange.start.line == -1) {
        if (codeRange.start.line == -1) {
            textRange.start.line = pos.line;
            textRange.end.line = pos.line;
        } else if (codeRange.start.line == pos.line) {
            textRange.start.line = codeRange.start.line;
            textRange.end.line = codeRange.end.line;
        } else {
            textRange.start.line = pos.line;
            textRange.end.line = codeRange.end.line;
        }
    } else {
        if (selRange.start.line == selRange.end.line
            && selRange.start.line == codeRange.start.line) {
            textRange.start.line = codeRange.start.line;
            textRange.end.line = codeRange.end.line;
        } else {
            textRange.start.line = selRange.start.line;
            textRange.end.line = selRange.end.line;
        }
    }

    return textRange;
}

QString InlineChatWidgetPrivate::addLineNumber(const QString &content, int start)
{
    QStringList lines = content.split('\n');
    for (int i = 0; i < lines.size(); ++i) {
        QString lineNumber = "%1  ";
        lines[i].prepend(lineNumber.arg(start++));
    }
    return lines.join('\n');
}

QString InlineChatWidgetPrivate::createFormatCode(const QString &file, const QString &code, const Edit::Range &range)
{
    if (state == QuestionStart)
        return code;

    codeInfo.isSelectEmpty = false;
    QString tempCode = code;
    tempCode.remove(QRegularExpression("\\s+"));
    if (tempCode.isEmpty()) {
        Edit::Range beforeRange = range;
        beforeRange.start.line -= 3;
        beforeRange.end.line -= 1;
        QString beforeText = editSrv->rangeText(file, beforeRange);

        Edit::Range laterRange = range;
        laterRange.start.line += 1;
        laterRange.end.line += 3;
        QString laterText = editSrv->rangeText(file, laterRange);

        codeInfo.isSelectEmpty = true;
        codeInfo.curosrLine = range.end.line + 1;

        QString format = "%1\n%2【cursor】\n%3";
        return addLineNumber(format.arg(beforeText, code, laterText), beforeRange.start.line + 1);
    }

    QString format = "%1【cursor】";
    return addLineNumber(format.arg(code), range.start.line + 1);
}

InlineChatWidget::InlineChatWidget(QWidget *parent)
    : QWidget(parent),
      d(new InlineChatWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

InlineChatWidget::~InlineChatWidget()
{
    delete d;
}

void InlineChatWidget::setLLM(AbstractLLM *llm)
{
    if (!llm)
        return;
    d->inlineChatLLM = llm;
}

void InlineChatWidget::start()
{
    reset();
    d->chatInfo.fileName = d->editSrv->currentFile();
    if (d->chatInfo.fileName.isEmpty())
        return;

    auto pos = d->editSrv->cursorPosition();
    const auto &textRange = d->calculateTextRange(d->chatInfo.fileName, pos);
    // TODO: Inline chat in the blank space
    auto rangeText = d->editSrv->rangeText(d->chatInfo.fileName, textRange);
    rangeText.remove(QRegularExpression("\\s+"));
    if (rangeText.isEmpty())
        return;

    d->editSrv->showLineWidget(textRange.start.line, this);
    d->defineBackgroundMarker(d->chatInfo.fileName);
    d->editSrv->setRangeBackgroundColor(d->chatInfo.fileName, textRange.start.line,
                                        textRange.end.line, d->selectionMarker);
}

void InlineChatWidget::reset()
{
    if (d->chatInfo.fileName.isEmpty())
        return;

    d->edit->clear();
    d->handleReject();
    if (d->selectionMarker != -1)
        d->editSrv->clearAllBackgroundColor(d->chatInfo.fileName, d->selectionMarker);
    d->prevState = InlineChatWidgetPrivate::Original;
    d->chatInfo.clear();
}

void InlineChatWidget::keyPressEvent(QKeyEvent *e)
{
    switch (e->modifiers()) {
    case Qt::NoModifier:
        if (e->key() == Qt::Key_Escape) {
            d->handleClose();
            return;
        }
    default:
        break;
    }
    QWidget::keyPressEvent(e);
}

bool InlineChatWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (obj != d->edit)
        return QWidget::eventFilter(obj, e);

    if (e->type() == QEvent::KeyPress) {
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
                } else if (d->stopBtn->isVisible()) {
                    d->handleStop();
                    return true;
                }
                break;
            default:
                break;
            }
        }
        case Qt::AltModifier: {
            switch (ke->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (d->edit->isVisible()) {
                    d->edit->insertPlainText("\n");
                    return true;
                }
                break;
            default:
                break;
            }
        }
        case Qt::KeypadModifier:
        case Qt::NoModifier: {
            switch (ke->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (d->submitBtn->isVisible())
                    d->handleSubmitEdit();
                return true;
            default:
                break;
            }
        } break;
        }
    } else if (e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut) {
        d->updateButtonIcon();
    } else if (e->type() == QEvent::InputMethod) {
        auto imEvent = static_cast<QInputMethodEvent *>(e);
        d->isPreinputStatus = imEvent->commitString().isEmpty();
    }

    return QWidget::eventFilter(obj, e);
}
