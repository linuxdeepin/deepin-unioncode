// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionitemwidget.h"

#include "services/session/sessionservice.h"
#include "common/settings/settings.h"

#include <DFontSizeManager>
#include <DDialog>
#include <DLineEdit>
#include <DIconButton>
#include <dboxwidget.h>

#include <QMouseEvent>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

ArrowHeaderLine::ArrowHeaderLine(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 0, 10, 0);
    mainLayout->setSpacing(0);

    DLabel *iconLabel = new DLabel(this);
    iconLabel->setPixmap(QIcon::fromTheme("uc_session").pixmap({ 20, 20 }));
    arrowButton = new DToolButton(this);
    arrowButton->setIcon(DStyle::standardIcon(style(), DStyle::SP_ReduceElement));
    arrowButton->setIconSize({ 12, 12 });
    arrowButton->setFixedSize(24, 24);
    titleLabel = new DLabel(this);
    titleLabel->installEventFilter(this);
    titleLabel->setCursor(Qt::PointingHandCursor);
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::T5, QFont::Medium);

    connect(arrowButton, &DToolButton::clicked, this, &ArrowHeaderLine::expandChanged);
    mainLayout->addWidget(iconLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(titleLabel, 1);
    mainLayout->addWidget(arrowButton);
    setFixedHeight(40);
    setExpand(false);
}

void ArrowHeaderLine::setExpand(bool value)
{
    if (value) {
        arrowButton->setIcon(DStyle::standardIcon(style(), DStyle::SP_ExpandElement));
    } else {
        arrowButton->setIcon(DStyle::standardIcon(style(), DStyle::SP_ReduceElement));
    }
    isExpanded = value;
}

void ArrowHeaderLine::setTitle(const QString &title)
{
    titleLabel->setText(title);
}

QString ArrowHeaderLine::title() const
{
    return titleLabel->text();
}

void ArrowHeaderLine::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::FontChange)
        setFixedHeight(qMax(EXPAND_HEADER_HEIGHT, this->fontMetrics().height()));

    QWidget::changeEvent(e);
}

bool ArrowHeaderLine::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == titleLabel) {
        switch (e->type()) {
        case QEvent::Enter: {
            QFont font = titleLabel->font();
            font.setUnderline(true);
            titleLabel->setFont(font);
            break;
        }
        case QEvent::Leave: {
            QFont font = titleLabel->font();
            font.setUnderline(false);
            titleLabel->setFont(font);
            break;
        }
        case QEvent::MouseButtonPress: {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
            if (mouseEvent->button() == Qt::LeftButton) {
                Q_EMIT itemClicked();
            }
            break;
        }
        default:
            break;
        }
    }

    return QWidget::eventFilter(obj, e);
}

void ArrowHeaderLine::reverseArrowDirection()
{
    setExpand(!isExpanded);
}

class SessionItemWidgetPrivate : public QObject
{
public:
    explicit SessionItemWidgetPrivate(SessionItemWidget *qq);

    void initUI();
    void initConnection();
    QWidget *createContent();
    DIconButton *createOptionButton(const QString &icon, const QString &tip);
    QString createProjectInfo(const QVariantList &projects);
    void removeSession();
    void renameSession();
    void cloneSession();
    void openSession();
    void runInputDialog(const QString &title, const QStringList &actList,
                        const QString &editText, std::function<void(const QString &)> handler);

public:
    SessionItemWidget *q;

    QString sessionName;
    SessionService *sessionSrv { nullptr };
    ArrowHeaderLine *headerLine { nullptr };
    DIconButton *cloneBtn { nullptr };
    DIconButton *renameBtn { nullptr };
    DIconButton *removeBtn { nullptr };
    DLabel *prjInfoLabel { nullptr };

    QPropertyAnimation *animation { nullptr };
    ContentBox *contentLoader { nullptr };
    DVBoxWidget *boxWidget { nullptr };
    bool expand { false };
};

SessionItemWidgetPrivate::SessionItemWidgetPrivate(SessionItemWidget *qq)
    : q(qq)
{
    sessionSrv = dpfGetService(SessionService);
}

void SessionItemWidgetPrivate::initUI()
{
    q->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    headerLine = new ArrowHeaderLine(q);
    contentLoader = new ContentBox(q);
    contentLoader->setFixedHeight(0);

    boxWidget = new DVBoxWidget(q);
    auto contentLayout = boxWidget->layout();
    contentLayout->addWidget(createContent());

    QVBoxLayout *loaderLayout = new QVBoxLayout(contentLoader);
    loaderLayout->setMargin(0);
    loaderLayout->setSpacing(0);
    loaderLayout->addWidget(boxWidget);
    loaderLayout->addStretch();

    animation = new QPropertyAnimation(contentLoader, "height", q);
    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::InSine);

    mainLayout->addWidget(headerLine);
    mainLayout->addWidget(contentLoader);
}

void SessionItemWidgetPrivate::initConnection()
{
    connect(headerLine, &ArrowHeaderLine::expandChanged, this, [this] {
        q->setExpand(!q->expand());
    });
    connect(boxWidget, &DVBoxWidget::sizeChanged, this, [this] {
        if (expand) {
            int endHeight = 0;
            endHeight = boxWidget->height();

            animation->setStartValue(contentLoader->height());
            animation->setEndValue(endHeight);
            animation->stop();
            animation->start();
        }
    });
    connect(animation, &QPropertyAnimation::valueChanged, this, [this] {
        q->setFixedHeight(q->sizeHint().height());
    });
    connect(headerLine, &ArrowHeaderLine::itemClicked, this, &SessionItemWidgetPrivate::openSession);
    connect(cloneBtn, &DIconButton::clicked, this, &SessionItemWidgetPrivate::cloneSession);
    connect(renameBtn, &DIconButton::clicked, this, &SessionItemWidgetPrivate::renameSession);
    connect(removeBtn, &DIconButton::clicked, this, &SessionItemWidgetPrivate::removeSession);
}

QWidget *SessionItemWidgetPrivate::createContent()
{
    QWidget *widget = new QWidget(q);
    QVBoxLayout *wLayout = new QVBoxLayout(widget);
    wLayout->setContentsMargins(0, 0, 0, 10);
    wLayout->setSpacing(8);

    prjInfoLabel = new DLabel(q);
    prjInfoLabel->setWordWrap(true);
    prjInfoLabel->setContentsMargins(40, 0, 0, 0);

    cloneBtn = createOptionButton("uc_clone", SessionItemWidget::tr("Clone"));
    renameBtn = createOptionButton("uc_edit", SessionItemWidget::tr("Rename"));
    removeBtn = createOptionButton("uc_delete", SessionItemWidget::tr("Remove"));

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(10);
    btnLayout->setContentsMargins(10, 0, 10, 0);
    btnLayout->setAlignment(Qt::AlignRight);
    btnLayout->addWidget(cloneBtn);
    btnLayout->addWidget(renameBtn);
    btnLayout->addWidget(removeBtn);

    wLayout->addWidget(prjInfoLabel);
    wLayout->addLayout(btnLayout);
    return widget;
}

DIconButton *SessionItemWidgetPrivate::createOptionButton(const QString &icon, const QString &tip)
{
    DIconButton *btn = new DIconButton(q);
    btn->setIconSize({ 16, 16 });
    btn->setIcon(QIcon::fromTheme(icon));
    btn->setToolTip(tip);
    btn->setFlat(true);
    return btn;
}

QString SessionItemWidgetPrivate::createProjectInfo(const QVariantList &projects)
{
    static QString formt("<div style=\"font-size: 14px; font-weight: 500;\">%1</div>"
                         "<div style=\"font-size: 12px; font-weight: 400;\">%2</div>");
    QString msg;
    for (const auto &p : projects) {
        const auto &map = p.toMap();
        QFileInfo info(map.value("Workspace").toString());
        msg += formt.arg(info.fileName(), info.absoluteFilePath());
    }

    return msg;
}

void SessionItemWidgetPrivate::removeSession()
{
    DDialog dlg(q);
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.setTitle(tr("Are you sure to remove this session?"));
    dlg.addButton(tr("Cancel", "button"));
    dlg.addButton(tr("Remove", "button"), true, DDialog::ButtonWarning);

    if (dlg.exec() == 1)
        sessionSrv->removeSession(sessionName);
}

void SessionItemWidgetPrivate::renameSession()
{
    QStringList actList { tr("Rename", "button"), tr("Rename and Open", "button") };
    runInputDialog(tr("Rename Session"), actList, sessionName,
                   [this](const QString &newName) {
                       sessionSrv->renameSession(sessionName, newName);
                   });
}

void SessionItemWidgetPrivate::cloneSession()
{
    QStringList actList { tr("Clone", "button"), tr("Clone and Open", "button") };
    runInputDialog(tr("New Session Name"), actList, sessionName + " (2)",
                   [this](const QString &newName) {
                       sessionSrv->cloneSession(sessionName, newName);
                   });
}

void SessionItemWidgetPrivate::openSession()
{
    sessionSrv->loadSession(sessionName);
}

void SessionItemWidgetPrivate::runInputDialog(const QString &title, const QStringList &actList,
                                              const QString &editText, std::function<void(const QString &)> handler)
{
    Q_ASSERT(actList.size() == 2);

    DDialog dlg(q);
    dlg.setSpacing(10);
    dlg.setTitle(title);
    dlg.setIcon(QIcon::fromTheme("ide"));
    DLineEdit *lineEdit = new DLineEdit(&dlg);
    lineEdit->setPlaceholderText(tr("Please input session name"));
    connect(lineEdit, &DLineEdit::textChanged, &dlg, [&dlg](const QString &text) {
        dlg.getButton(1)->setEnabled(!text.isEmpty());
        dlg.getButton(2)->setEnabled(!text.isEmpty());
    });
    dlg.addContent(lineEdit);

    dlg.addButton(tr("Cancel", "button"));
    dlg.addButton(actList[0]);
    dlg.addButton(actList[1], true, DDialog::ButtonRecommend);
    dlg.getButton(1)->setEnabled(false);
    dlg.getButton(2)->setEnabled(false);
    lineEdit->setText(editText);

    int ret = dlg.exec();
    if (ret < 1)
        return;

    const auto name = lineEdit->text();
    if (name.isEmpty() || sessionSrv->sessionList().contains(name))
        return;

    handler(name);
    if (ret == 2)
        sessionSrv->loadSession(name);
}

SessionItemWidget::SessionItemWidget(QWidget *parent)
    : DFrame(parent),
      d(new SessionItemWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

SessionItemWidget::~SessionItemWidget()
{
    delete d;
}

void SessionItemWidget::setSessionName(const QString &session)
{
    d->sessionName = session;
}

QString SessionItemWidget::sessionName() const
{
    return d->sessionName;
}

void SessionItemWidget::setExpand(bool value)
{
    if (d->expand == value)
        return;

    d->headerLine->setExpand(value);
    d->expand = value;
    if (value) {
        d->animation->setStartValue(0);
        d->animation->setEndValue(d->boxWidget->height());
    } else {
        d->animation->setStartValue(d->boxWidget->height());
        d->animation->setEndValue(0);
    }

    d->animation->stop();
    d->animation->start();
}

bool SessionItemWidget::expand() const
{
    return d->expand;
}

void SessionItemWidget::updateSession()
{
    bool isLastSession = d->sessionSrv->lastSession() == d->sessionName;
    bool isCurrentSession = d->sessionSrv->currentSession() == d->sessionName;
    bool isDefaultSession = d->sessionSrv->isDefaultSession(d->sessionName);
    bool isDefaultVirgin = d->sessionSrv->isDefaultVirgin();

    d->renameBtn->setEnabled(!isDefaultSession);
    d->removeBtn->setEnabled(!isDefaultSession && !isCurrentSession);

    auto title = d->sessionName;
    if (isLastSession && isDefaultVirgin)
        title = tr("%1 (last session)").arg(title);
    if (isCurrentSession && !isDefaultVirgin)
        title = tr("%1 (current session)").arg(title);
    d->headerLine->setTitle(title);

    const auto &sessionCfg = d->sessionSrv->sessionFile(d->sessionName);
    if (!QFile::exists(sessionCfg))
        return d->prjInfoLabel->setVisible(false);

    Settings st("", sessionCfg);
    const auto &prjList = st.value("Session", "ProjectList").toList();
    const auto &info = d->createProjectInfo(prjList);
    d->prjInfoLabel->setVisible(!info.isEmpty());
    d->prjInfoLabel->setText(info);
}

void SessionItemWidget::resizeEvent(QResizeEvent *e)
{
    if (d->contentLoader)
        d->contentLoader->setFixedWidth(e->size().width());

    if (d->headerLine)
        d->headerLine->setFixedWidth(e->size().width());
    DFrame::resizeEvent(e);
}

bool SessionItemWidget::eventFilter(QObject *obj, QEvent *e)
{
    return DFrame::eventFilter(obj, e);
}

SessionItemListWidget::SessionItemListWidget(QWidget *parent)
    : QScrollArea(parent)
{
    QWidget *widget = new QWidget(this);
    mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);
    mainLayout->setAlignment(Qt::AlignTop);

    setFrameShape(QFrame::NoFrame);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    setWidgetResizable(true);
    setWidget(widget);
}

void SessionItemListWidget::addSessionList(const QStringList &sessionList)
{
    for (const auto &session : sessionList) {
        auto item = new SessionItemWidget(this);
        item->setSessionName(session);
        this->sessionList.append(item);
        mainLayout->addWidget(item);
    }
    updateSessions();
}

void SessionItemListWidget::removeSession(const QString &session)
{
    for (auto *item : sessionList) {
        if (item->sessionName() == session) {
            sessionList.removeOne(item);
            mainLayout->removeWidget(item);
            item->deleteLater();
            item = nullptr;
            updateSessions();
            break;
        }
    }
}

void SessionItemListWidget::updateSessions()
{
    for (int i = 0; i < sessionList.size(); ++i) {
        auto item = sessionList[i];
        item->updateSession();
        item->setFixedWidth(width());
        if (i % 2 == 0) {
            item->setBackgroundRole(DPalette::ItemBackground);
        } else {
            item->setBackgroundRole(QPalette::Base);
        }
    }
}

void SessionItemListWidget::resizeEvent(QResizeEvent *e)
{
    for (auto *item : qAsConst(sessionList)) {
        item->setFixedWidth(e->size().width());
    }
    QScrollArea::resizeEvent(e);
}
