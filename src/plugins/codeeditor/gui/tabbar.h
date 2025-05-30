// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABBAR_H
#define TABBAR_H

#include <QWidget>

class TabBarPrivate;
class TabBar : public QWidget
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = nullptr);

    void setFileName(const QString &fileName);
    int indexOf(const QString &fileName);
    QString indexFile(int index) const;
    int tabCount() const;
    int currentIndex() const;
    QString currentFileName() const;
    void setCurrentIndex(int index);

    void switchTab(const QString &fileName);
    void removeTab(const QString &fileName, bool silent = false);
    void closeAllTab(const QStringList &exceptList, bool silent = false);

    void setCloseButtonVisible(bool visible);
    void setSplitButtonVisible(bool visible);

public slots:
    void closeTab(int index);
    void onModificationChanged(const QString &fileName, bool isModified);

signals:
    void spliterClicked(Qt::Orientation ori);
    void closeRequested();
    void saveFileRequested(const QString &fileName);
    void tabClosed(const QString &fileName);
    void tabSwitched(const QString &fileName);

private:
    QSharedPointer<TabBarPrivate> d { nullptr };
};

#endif   // TABBAR_H
