// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODECOMPLETIONWIDGET_H
#define CODECOMPLETIONWIDGET_H

#include <QFrame>

class TextEditor;
class CodeCompletionView;
class CodeCompletionModel;
class CompletionSortFilterProxyModel;
class CodeCompletionExtendWidget;

class CodeCompletionWidget : public QFrame
{
    Q_OBJECT
public:
    explicit CodeCompletionWidget(TextEditor *parent);

    TextEditor *editor() const;

    bool isCompletionActive() const;
    void startCompletion();

    void updateHeight();
    void updatePosition(bool force = false);
    void setCompletion(const QString &info, const QIcon &icon, const QKeySequence &key);

public slots:
    bool processKeyPressEvent(QKeyEvent *event);
    bool execute();

    void abortCompletion();
    void automaticInvocation();

protected:
    virtual void focusOutEvent(QFocusEvent *event) override;

private:
    void initUI();
    void initConnection();

    bool shouldStartCompletion(const QString &insertedText);
    void updateAndShow();
    bool hasAtLeastNRows(int rows);
    QString filterString();
    bool isFunctionKind(int kind);
    void executeCompletionItem(const QModelIndex &index);

private slots:
    void modelContentChanged();
    void onCompletionChanged();
    void onTextAdded(int pos, int len, int added, const QString &text, int line);
    void onTextRemoved(int pos, int len, int added, const QString &text, int line);
    void viewFocusOut();
    void cursorPositionChanged();

private:
    CodeCompletionView *completionView { nullptr };
    CodeCompletionModel *completionModel { nullptr };
    CompletionSortFilterProxyModel *proxyModel { nullptr };
    CodeCompletionExtendWidget *completionExtWidget { nullptr };

    QTimer *automaticInvocationTimer { nullptr };

    QString automaticInvocationLine;
    int automaticInvocationAt;

    bool needShow { false };
    bool isCompletionInput { false };
};

#endif   // CODECOMPLETIONWIDGET_H
