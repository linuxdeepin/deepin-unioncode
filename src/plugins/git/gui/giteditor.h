// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITEDITOR_H
#define GITEDITOR_H

#include <QPlainTextEdit>

class GitEditorPrivate;
class GitEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit GitEditor(QWidget *parent = nullptr);
    ~GitEditor();

    void setSourceFile(const QString &sf);
    QString sourceFile() const;

    int extraAreaWidth() const;
    void extraAreaPaintEvent(QPaintEvent *e);

    QString changeUnderCursor(const QTextCursor &c);

protected:
    QString lineNumber(int blockNumber) const;
    int lineNumberDigits() const;

    void resizeEvent(QResizeEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    GitEditorPrivate *const d;
    friend class GitEditorPrivate;
};

#endif // GITEDITOR_H
