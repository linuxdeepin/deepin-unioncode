// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEEDITCOMPONENT_H
#define CODEEDITCOMPONENT_H


#include <QWidget>

QT_BEGIN_NAMESPACE
class QPushButton;
class QPlainTextEdit;
class QLabel;
QT_END_NAMESPACE

class CodeEditComponent : public QWidget
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

    void setUpdateHeight(bool update);
    void updateCode(const QString &code);
    void updateCode(const QStringList &codeLines);

    void cleanFinalLine();

    QString getContent();

public Q_SLOTS:
    void onInsertBtnClicked();
    void onCopyBtnClicked();

private:
    void initUI();
    void initTitleWidgets();
    void initConnection();

    QWidget *titleWidget { nullptr };
    QPushButton *copyButton { nullptr };
    QPushButton *insertButton { nullptr };
    QPlainTextEdit *codeEdit { nullptr };
    QLabel *title { nullptr };

    bool heightUpdate { false };
};

#endif // CODEEDITCOMPONENT_H
