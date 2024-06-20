// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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

    int extraAreaWidth() const;
    void extraAreaPaintEvent(QPaintEvent *e);

protected:
    QString lineNumber(int blockNumber) const;
    int lineNumberDigits() const;

    void resizeEvent(QResizeEvent *e) override;

private:
    GitEditorPrivate *const d;
    friend class GitEditorPrivate;
};

#endif // GITEDITOR_H
