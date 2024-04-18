// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEEDITCOMPONENT_H
#define CODEEDITCOMPONENT_H
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/syntaxhighlighter.h>
#include <KSyntaxHighlighting/theme.h>

#include <DWidget>
#include <DLabel>
#include <DTextEdit>
#include <DFrame>
#include <DToolButton>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QPushButton;
class QPlainTextEdit;
class QLabel;
QT_END_NAMESPACE

class CodeEditComponent : public DWidget
{
    Q_OBJECT
public:
    enum ShowButtonsType {
        None,
        CopyOnly,
        InsertOnly,
        CopyAndInsert
    };

    explicit CodeEditComponent(QWidget *parent = nullptr);
    void showButtons(ShowButtonsType type);
    void setTitle(const QString &title);
    void setReadOnly(bool readOnly);
    void setPlaceholderText(const QString &text);

    void setUpdateHeight(bool update);
    void updateCode(const QString &code, const QString &language = "C++");
    void updateCode(const QStringList &codeLines, const QString &language = "C++");

    void cleanFinalLine();

    QString getContent();

public Q_SLOTS:
    void onInsertBtnClicked();
    void onCopyBtnClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initUI();
    void initTitleWidgets();
    void initButton();
    void initConnection();
    void updateDefinition(const QString &language);

    DWidget *titleWidget { nullptr };
    DToolButton *copyButton { nullptr };
    DToolButton *insertButton { nullptr };
    DFrame *editFrame { nullptr };
    DHorizontalLine *hLine { nullptr };
    QTextEdit *codeEdit { nullptr };
    DLabel *title { nullptr };

    KSyntaxHighlighting::SyntaxHighlighter *highLighter { nullptr };
    KSyntaxHighlighting::Repository rep;
    KSyntaxHighlighting::Definition def;

    bool heightUpdate { false };
};

#endif   // CODEEDITCOMPONENT_H
