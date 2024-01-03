// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITTABBAR_H
#define TEXTEDITTABBAR_H

#include <DTabBar>
#include <DTabWidget>

class TextEditTabBarPrivate;
class TextEditTabBar : public DTK_WIDGET_NAMESPACE::DTabWidget
{
    Q_OBJECT
    friend class TextEditTabBarPrivate;
    TextEditTabBarPrivate *const d;
public:
    explicit TextEditTabBar(QWidget *parent = nullptr);
    virtual ~TextEditTabBar();
    int fileIndex(const QString &file) const;
    QString indexFile(int index) const;
    void setFile(const QString &file);
    void switchFile(const QString &file);
    void removeTab(const QString &file);
    int count() const;
    int currentIndex() const;
    QString currentFile() const;
    void setCurrentIndex(int idx);
    void setCloseButtonVisible(bool flag);
    void setSplitButtonVisible(bool flag);

signals:
    void fileClosed(const QString &file);
    void fileSwitched(const QString &file);
    void saveFile(const QString &file);
    void splitClicked(Qt::Orientation ori);
    void closeClicked();

public slots:
    void tabCloseRequested(int idx);
    void doFileChanged(const QString &file);
    void doFileSaved(const QString &file);
    void showMenu(QPoint pos);
};

#endif // TEXTEDITTABBAR_H
