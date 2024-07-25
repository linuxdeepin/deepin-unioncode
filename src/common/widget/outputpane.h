// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OUTPUTPANE_H
#define OUTPUTPANE_H

#include <QWidget>
#include <QPlainTextEdit>

class OutputPanePrivate;
class OutputPane : public QWidget
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
    QTextDocument *document() const;
    QPlainTextEdit *edit() const;

    static OutputPane* instance();

    void updateFilter(const QString &filterText, bool caseSensitive = false, bool isRegexp = false); // support all param later
    void filterContent(bool caseSensitive = false, bool isRegexp = false);
protected:
    void contextMenuEvent(QContextMenuEvent * event) override;

private:
    void initUI();
    void initTimer();
    QWidget *createFindPlaceHolder();
    QString normalizeNewlines(const QString &text);

    void handleNextOutput();
    void appendCustomText(const QString &text, AppendMode mode, const QTextCharFormat &format = QTextCharFormat());
    bool isScrollbarAtBottom() const;
    QString doNewlineEnforcement(const QString &out);
    void scrollToBottom();
    QList<QAction*> actionFactory();

    OutputPanePrivate *d = nullptr;
};

#endif // OUTPUTPANE_H
