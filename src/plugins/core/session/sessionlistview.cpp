// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionlistview.h"
#include "sessiondialog.h"
#include "sessionmanager.h"

#include "base/baseitemdelegate.h"

#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

class SessionItemDelegate : public BaseItemDelegate
{
public:
    SessionItemDelegate(QObject *parent = nullptr)
        : BaseItemDelegate(parent) {}

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

void SessionItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing);
    QStyleOptionViewItem opt = option;
    opt.state &= ~QStyle::State_HasFocus;
    BaseItemDelegate::paint(painter, opt, index);
}

QSize SessionItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    auto rect = option.rect;
    return { rect.width(), 30 };
}

SessionListView::SessionListView(QWidget *parent)
    : QTreeView(parent)
{
    initUI();
    initConnections();
}

void SessionListView::initUI()
{
    setUniformRowHeights(true);
    setItemDelegate(new SessionItemDelegate(this));
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setWordWrap(false);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setModel(&model);
    sortByColumn(0, Qt::AscendingOrder);
    header()->setSectionResizeMode(QHeaderView::Stretch);
}

void SessionListView::initConnections()
{
    connect(this, &SessionListView::activated, this, &SessionListView::switchToCurrentSession);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
        Q_EMIT sessionsSelected(selectedSessions());
    });

    connect(&model, &SessionModel::modelReset, this, &SessionListView::selectActiveSession);
    connect(this, &SessionListView::sessionCreated, this, &SessionListView::selectSession);
}

QString SessionListView::currentSession() const
{
    return model.sessionAt(selectionModel()->currentIndex().row());
}

void SessionListView::selectSession(const QString &session)
{
    int row = model.rowOfSession(session);
    selectionModel()->setCurrentIndex(model.index(row, 0),
                                      QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void SessionListView::selectActiveSession()
{
    selectSession(SessionManager::instance()->currentSession());
}

void SessionListView::createSession()
{
    SessionNameInputDialog dlg;
    dlg.setTitle(tr("New Session Name"));
    dlg.setActionText(tr("Create", "button"),
                      tr("Create and Open", "button"));

    runInputDialog(&dlg, [](const QString &newName) {
        SessionManager::instance()->createSession(newName);
    });
}

void SessionListView::removeSelectedSessions()
{
    const auto &sessions = selectedSessions();

    DDialog dlg(this);
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    if (sessions.size() > 1)
        dlg.setTitle(tr("Are you sure to delete these sessions?"));
    else
        dlg.setTitle(tr("Are you sure to remove this session?"));
    dlg.addButton(tr("Cancel", "button"));
    dlg.addButton(tr("Remove", "button"), true, DDialog::ButtonWarning);

    if (dlg.exec() == 1) {
        SessionManager::instance()->removeSessions(selectedSessions());
        model.reset();
    }
}

void SessionListView::cloneCurrentSession()
{
    SessionNameInputDialog dlg;
    dlg.setTitle(tr("New Session Name"));
    dlg.setActionText(tr("Clone", "button"),
                      tr("Clone and Open", "button"));
    const auto session = currentSession();
    dlg.setSessionName(session + " (2)");

    runInputDialog(&dlg, [session](const QString &newName) {
        SessionManager::instance()->cloneSession(session, newName);
    });
}

void SessionListView::renameCurrentSession()
{
    SessionNameInputDialog dlg;
    dlg.setTitle(tr("Rename Session"));
    dlg.setActionText(tr("Rename", "button"),
                      tr("Rename and Open", "button"));
    const auto session = currentSession();
    dlg.setSessionName(session);

    runInputDialog(&dlg, [session](const QString &newName) {
        SessionManager::instance()->renameSession(session, newName);
    });
}

void SessionListView::switchToCurrentSession()
{
    const auto session = currentSession();
    SessionManager::instance()->loadSession(session);
    Q_EMIT sessionSwitched();
}

void SessionListView::showEvent(QShowEvent *event)
{
    QTreeView::showEvent(event);
    selectActiveSession();
    setFocus();
}

void SessionListView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() != Qt::Key_Delete && event->key() != Qt::Key_Backspace) {
        QTreeView::keyPressEvent(event);
        return;
    }
    const QStringList sessions = selectedSessions();
    bool ret = std::any_of(sessions.cbegin(), sessions.cend(),
                           [](const QString &session) {
                               return session == SessionManager::instance()->currentSession();
                           });
    if (!sessions.contains("default") && !ret)
        removeSelectedSessions();
}

QStringList SessionListView::selectedSessions() const
{
    const auto &indexList = selectionModel()->selectedRows();
    QStringList selectedSessions;
    std::transform(indexList.cbegin(), indexList.cend(), std::back_inserter(selectedSessions),
                   [this](const QModelIndex &index) {
                       return model.sessionAt(index.row());
                   });
    return selectedSessions;
}

void SessionListView::runInputDialog(SessionNameInputDialog *dialog,
                                     std::function<void(const QString &)> handler)
{
    if (dialog->exec() == QDialog::Accepted) {
        const auto name = dialog->sessionName();
        if (name.isEmpty())
            return;

        handler(name);
        model.reset();
        if (dialog->isSwitchToRequested()) {
            SessionManager::instance()->loadSession(name);
            Q_EMIT sessionSwitched();
        }
        Q_EMIT sessionCreated(name);
    }
}
