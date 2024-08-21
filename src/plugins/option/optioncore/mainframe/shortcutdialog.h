// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DDialog>

DWIDGET_BEGIN_NAMESPACE
class DLineEdit;
class DLabel;
DWIDGET_END_NAMESPACE

class ShortcutItem;
class ShortcutDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit ShortcutDialog(QWidget *parent = nullptr);

    QKeySequence keySequece() const;
    using ConflictCheckHandler = std::function<int(const QKeySequence &)>;
    void setConflictCheckHandler(ConflictCheckHandler handler);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initUI();
    void keyValueChanged();
    void resetState();
    int translateModifiers(Qt::KeyboardModifiers state, const QString &text);

    DTK_WIDGET_NAMESPACE::DLineEdit *keyEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *msgLabel { nullptr };
    ConflictCheckHandler checkHandler { nullptr };
    QKeySequence shortcutKey;
    std::array<int, 4> keyArray { 0 };
    int keyNum { 0 };
};
