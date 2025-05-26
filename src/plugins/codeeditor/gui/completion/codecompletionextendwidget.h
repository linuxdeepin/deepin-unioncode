// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODECOMPLETIONEXTENDWIDGET_H
#define CODECOMPLETIONEXTENDWIDGET_H

#include <QWidget>

class TextEditor;
class CodeCompletionExtendWidgetPrivate;
class CodeCompletionExtendWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CodeCompletionExtendWidget(QWidget *parent = nullptr);
    ~CodeCompletionExtendWidget();

    void setCompletionInfo(const QString &info, const QIcon &icon, const QKeySequence &key);
    bool processEvent(QKeyEvent *event);
    bool isValid();
    void setTextEditor(TextEditor *editor);

signals:
    void completionChanged();

protected:
    virtual void hideEvent(QHideEvent *event) override;

private:
    CodeCompletionExtendWidgetPrivate *const d;
};

#endif   // CODECOMPLETIONEXTENDWIDGET_H
