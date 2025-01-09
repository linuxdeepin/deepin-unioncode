// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONDIALOG_H
#define SESSIONDIALOG_H

#include <DDialog>
#include <DLineEdit>
#include <DIconButton>
#include <DPushButton>
#include <DLabel>

class SessionListView;
class SessionDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit SessionDialog(QWidget *parent = nullptr);

    void setAutoLoadSession(bool autoLoad);
    bool autoLoadSession() const;

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    void initUI();
    void initConnect();
    QWidget *createOptionWidget();
    void updateOptions(const QStringList &sessions);

private:
    SessionListView *view { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *addBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *openBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *renameBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *cloneBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *removeBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DCheckBox *autoLoadCB { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *helpLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *closeBtn { nullptr };
};

class SessionNameInputDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit SessionNameInputDialog(QWidget *parent = nullptr);

    void setSessionName(const QString &name);
    QString sessionName() const;
    void setActionText(const QString &actText, const QString &openActText);
    bool isSwitchToRequested() const;

private:
    void initUI();
    void handleButtonClicked(int index);

    DTK_WIDGET_NAMESPACE::DLineEdit *lineEdit { nullptr };
    bool isSwitchTo { false };
};

#endif   // SESSIONDIALOG_H
