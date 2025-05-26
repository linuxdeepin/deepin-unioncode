// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONITEMWIDGET_H
#define SESSIONITEMWIDGET_H

#include <DDrawer>
#include <DToolButton>
#include <DLabel>
#include <dbaseline.h>

#include <QScrollArea>

class ContentBox : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int height READ height WRITE setFixedHeight)
public:
    explicit ContentBox(QWidget *parent = nullptr)
        : QWidget(parent)
    {
    }
};

class ArrowHeaderLine : public QWidget
{
    Q_OBJECT
public:
    ArrowHeaderLine(QWidget *parent = nullptr);
    void setExpand(bool value);
    void setTitle(const QString &title);
    QString title() const;
    void setTitleTip(const QString &tooltip);

Q_SIGNALS:
    void expandChanged();
    void itemClicked();

protected:
    void resizeEvent(QResizeEvent *e) override;
    void changeEvent(QEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    void reverseArrowDirection();
    void updateTitle();

    bool isExpanded { false };
    QString titleText;
    DTK_WIDGET_NAMESPACE::DToolButton *arrowButton { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *titleLabel { nullptr };
};

class SessionItemWidgetPrivate;
class SessionItemWidget : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit SessionItemWidget(QWidget *parent = nullptr);
    ~SessionItemWidget();

    void setSessionName(const QString &session);
    QString sessionName() const;
    void setExpand(bool value);
    bool expand() const;
    void updateSession();

private:
    void resizeEvent(QResizeEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

    SessionItemWidgetPrivate *const d;
};

class SessionItemListWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit SessionItemListWidget(QWidget *parent = nullptr);

    void addSessionList(const QStringList &sessionList);
    void removeSession(const QString &session);
    void updateSessions();

private:
    void resizeEvent(QResizeEvent *e) override;

    QList<SessionItemWidget *> sessionList;
    QVBoxLayout *mainLayout { nullptr };
};

#endif   // SESSIONITEMWIDGET_H
