// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORSTATUSBAR_H
#define EDITORSTATUSBAR_H

#include <QWidget>

class EditorStatusBarPrivate;
class EditorStatusBar : public QWidget
{
    Q_OBJECT
public:
    enum CursorMode {
        Insert,
        Overwrite,
        Readonly
    };

    explicit EditorStatusBar(QWidget *parent = nullptr);
    ~EditorStatusBar();

public Q_SLOTS:
    void updateCursor(int line, int column);
    void updateCursorMode(CursorMode mode);
    void updateEncodedMode(const QString &mode);

Q_SIGNALS:
    void encodeChanged(const QString &encode);

private:
    EditorStatusBarPrivate *const d;
};

#endif   // EDITORSTATUSBAR_H
