// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEEDITCOMPONENT_H
#define CODEEDITCOMPONENT_H

#include <DWidget>
#include <DLabel>
#include <DTextEdit>
#include <DFrame>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Definition>

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
    void updateCode(const QString &code);
    void updateCode(const QStringList &codeLines);

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

    DWidget *titleWidget { nullptr };
    DPushButton *copyButton { nullptr };
    DPushButton *insertButton { nullptr };
    DFrame *editFrame { nullptr };
    DHorizontalLine *hLine { nullptr };
    DTextEdit *codeEdit { nullptr };
    DLabel *title { nullptr };

    KSyntaxHighlighting::Repository rep;
    KSyntaxHighlighting::Definition def;

    bool heightUpdate { false };
};

#endif   // CODEEDITCOMPONENT_H
