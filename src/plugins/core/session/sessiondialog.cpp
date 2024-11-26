// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessiondialog.h"
#include "sessionlistview.h"
#include "sessionmanager.h"

#include <DFrame>
#include <DStyle>
#include <DCheckBox>

#include <QPushButton>
#include <QVBoxLayout>
#include <QEvent>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

SessionDialog::SessionDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
    initConnect();
}

void SessionDialog::initUI()
{
    setFixedSize(550, 440);
    setIcon(QIcon::fromTheme("ide"));
    setWindowTitle(tr("Session Manager"));
    setContentLayoutContentsMargins(QMargins(20, 0, 20, 0));

    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    DFrame *frame = new DFrame(this);
    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(3, 3, 3, 3);
    layout->setSpacing(0);

    view = new SessionListView(frame);
    view->setFrameShape(QFrame::NoFrame);
    view->setAlternatingRowColors(true);
    autoLoadCB = new DCheckBox(tr("Restore last session on startup"), this);
    helpLabel = new DLabel(this);
    helpLabel->installEventFilter(this);
    helpLabel->setWordWrap(true);
    helpLabel->setFixedSize(36, 36);
    helpLabel->setAlignment(Qt::AlignCenter);
    helpLabel->setPixmap(QIcon::fromTheme("uc_help").pixmap(16, 16));
    QString tipFormat("<div style='width: 300px;'><p>%1</p><p>%2</p></div>");
    helpLabel->setToolTip(tipFormat.arg(tr("Session management is a crucial feature in modern IDEs that allows developers "
                                           "to preserve and restore their working environment across different coding sessions."),
                                        tr("This functionality significantly improves productivity by eliminating the need "
                                           "to manually reconstruct working environments and allowing developers to maintain "
                                           "separate contexts for different tasks or projects.")));
    closeBtn = new DPushButton(tr("Close", "button"), this);
    closeBtn->setMinimumWidth(170);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addWidget(helpLabel, 0, Qt::AlignLeft);
    btnLayout->addWidget(closeBtn, 0, Qt::AlignRight);

    layout->addWidget(view, 1);
    layout->addWidget(new DHorizontalLine(frame));
    layout->addWidget(createOptionWidget());

    mainLayout->addWidget(frame);
    mainLayout->addWidget(autoLoadCB, 0, Qt::AlignLeft);
    mainLayout->addLayout(btnLayout);

    addContent(contentWidget);
}

void SessionDialog::initConnect()
{
    connect(view, &SessionListView::sessionsSelected, this, &SessionDialog::updateOptions);
    connect(addBtn, &DIconButton::clicked, view, &SessionListView::createSession);
    connect(renameBtn, &DIconButton::clicked, view, &SessionListView::renameCurrentSession);
    connect(cloneBtn, &DIconButton::clicked, view, &SessionListView::cloneCurrentSession);
    connect(removeBtn, &DIconButton::clicked, view, &SessionListView::removeSelectedSessions);
    connect(openBtn, &DIconButton::clicked, view, &SessionListView::switchToCurrentSession);
    connect(closeBtn, &DPushButton::clicked, this, &SessionDialog::close);
}

QWidget *SessionDialog::createOptionWidget()
{
    auto createButton = [this](const QString &icon, const QString &tip) {
        DIconButton *iconBtn = new DIconButton(this);
        iconBtn->setIcon(QIcon::fromTheme(icon));
        iconBtn->setIconSize({ 16, 16 });
        iconBtn->setToolTip(tip);
        iconBtn->setFlat(true);
        return iconBtn;
    };

    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignLeft);
    layout->setContentsMargins(7, 3, 3, 3);
    layout->setSpacing(10);

    addBtn = createButton("uc_add", tr("New Session"));
    openBtn = createButton("uc_open", tr("Open Session"));
    renameBtn = createButton("uc_edit", tr("Rename Session"));
    cloneBtn = createButton("uc_clone", tr("Clone Session"));
    removeBtn = createButton("uc_delete", tr("Remove Session"));

    DVerticalLine *vLine = new DVerticalLine(this);
    vLine->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->addWidget(addBtn);
    layout->addWidget(vLine);
    layout->addWidget(openBtn);
    layout->addWidget(renameBtn);
    layout->addWidget(cloneBtn);
    layout->addWidget(removeBtn);

    return widget;
}

void SessionDialog::setAutoLoadSession(bool autoLoad)
{
    autoLoadCB->setChecked(autoLoad);
}

bool SessionDialog::autoLoadSession() const
{
    return autoLoadCB->isChecked();
}

bool SessionDialog::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == helpLabel && e->type() == QEvent::Paint) {
        QPainter painter(helpLabel);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addRoundedRect(helpLabel->rect(), 8, 8);

        painter.setClipPath(path);
        painter.fillRect(helpLabel->rect(), helpLabel->palette().button());
    }

    return DDialog::eventFilter(obj, e);
}

void SessionDialog::updateOptions(const QStringList &sessions)
{
    if (sessions.isEmpty()) {
        openBtn->setEnabled(false);
        renameBtn->setEnabled(false);
        cloneBtn->setEnabled(false);
        removeBtn->setEnabled(false);
        return;
    }

    bool defaultIsSelected = sessions.contains("default");
    bool activeIsSelected = std::any_of(sessions.cbegin(), sessions.cend(),
                                        [](const QString &session) {
                                            return session == SessionManager::instance()->currentSession();
                                        });
    openBtn->setEnabled(sessions.size() == 1);
    renameBtn->setEnabled(sessions.size() == 1 && !defaultIsSelected);
    cloneBtn->setEnabled(sessions.size() == 1);
    removeBtn->setEnabled(!defaultIsSelected && !activeIsSelected);
}

SessionNameInputDialog::SessionNameInputDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
}

void SessionNameInputDialog::initUI()
{
    setSpacing(10);
    setIcon(QIcon::fromTheme("ide"));
    lineEdit = new DLineEdit(this);
    lineEdit->setPlaceholderText(tr("Please input session name"));
    connect(lineEdit, &DLineEdit::textChanged, this, [this](const QString &text) {
        getButton(1)->setEnabled(!text.isEmpty());
        getButton(2)->setEnabled(!text.isEmpty());
    });
    addContent(lineEdit);

    addButton(tr("Cancel", "button"));
    addButton("2");
    addButton("3", true, DDialog::ButtonRecommend);
    getButton(1)->setEnabled(false);
    getButton(2)->setEnabled(false);
    setFocusProxy(lineEdit);
}

void SessionNameInputDialog::setSessionName(const QString &name)
{
    lineEdit->setText(name);
}

QString SessionNameInputDialog::sessionName() const
{
    return lineEdit->text();
}

void SessionNameInputDialog::setActionText(const QString &actText, const QString &openActText)
{
    getButton(1)->setText(actText);
    getButton(2)->setText(openActText);
}
