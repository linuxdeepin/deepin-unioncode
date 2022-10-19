/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#ifndef TASKWINDOW_H
#define TASKWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QAction;
class QModelIndex;
class QPoint;
QT_END_NAMESPACE

namespace ReverseDebugger {
namespace Internal {
class Task;
class TaskWindowPrivate;

class TaskWindow : public QObject
{
    Q_OBJECT
public:
    explicit TaskWindow();
    ~TaskWindow();

    void delayedInitialization();

    int taskCount(const QString &category = "") const;

    // IOutputPane
    QWidget *outputWidget() const;
    QList<QWidget *> toolBarWidgets() const;

    QString displayName() const { return tr("Events"); }
    int priorityInStatusBar() const;
    void clearContents();
    void visibilityChanged(bool visible);

    bool canFocus() const;
    bool hasFocus() const;
    void setFocus();

    bool canNavigate() const;
    bool canNext() const;
    bool canPrevious() const;
    void goToNext();
    void goToPrev();
    void goTo(int index);
    void addTask(const Task &task);
    void removeTask(const Task &task);
    void addCategory(const QString &categoryId, const QString &displayName, bool visible);
    void updateTimeline(void* timeline, int count);

signals:
    void tasksChanged();
    void tasksCleared();
    void coredumpChanged(int index);

    // parent
    void setBadgeNumber(int number);
    void navigateStateUpdate();

public slots:
    void execCommand(void);
    void sortEvent(int index);

    // parent
    void navigateStateChanged() { emit navigateStateUpdate(); }

private:
    void showTask(unsigned int id);
    void openTask(unsigned int id);
    void clearTasks(const QString &categoryId);
    void setCategoryVisibility(const QString &categoryId, bool visible);
    void currentChanged(const QModelIndex &index);
    void saveSettings();
    void loadSettings();

    void triggerDefaultHandler(const QModelIndex &index);
    void clickItem(const QModelIndex &index);
    void actionTriggered();
    void updateCategoriesMenu();

    TaskWindowPrivate *const d;
};

} // namespace Internal
} // namespace ReverseDebugger

#endif // TASKWINDOW_H
