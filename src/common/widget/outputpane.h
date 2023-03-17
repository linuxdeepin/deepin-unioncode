/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#ifndef OUTPUTPANE_H
#define OUTPUTPANE_H

#include <QPlainTextEdit>

class OutputPanePrivate;
class OutputPane : public QPlainTextEdit
{
    Q_OBJECT
public:

    enum OutputFormat {
        NormalMessage,
        ErrorMessage,
        LogMessage,
        Debug,
        StdOut,
        StdErr,
        StdOutFormatSameLine,
        StdErrFormatSameLine,
        NumberOfFormats   // Keep this entry last.
    };

    enum AppendMode {
        Normal,
        OverWrite
    };

    OutputPane(QWidget *parent = nullptr);
    ~OutputPane() override;

    void clearContents();
    void appendText(const QString &text, OutputPane::OutputFormat format, AppendMode mode = Normal);
    static OutputPane* instance();

protected:
    void contextMenuEvent(QContextMenuEvent * event) override;

private:
    QString normalizeNewlines(const QString &text);

    void appendCustomText(const QString &text, AppendMode mode, const QTextCharFormat &format = QTextCharFormat());
    bool isScrollbarAtBottom() const;
    QString doNewlineEnforcement(const QString &out);
    void scrollToBottom();
    QList<QAction*> actionFactory();

    OutputPanePrivate *d = nullptr;
};

#endif // OUTPUTPANE_H
