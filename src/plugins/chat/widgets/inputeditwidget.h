// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTEDITWIDGET_H
#define INPUTEDITWIDGET_H

#include <DFrame>
#include <DTextEdit>

#include <QEvent>

class TagObjectInterface;
class TagTextFormat : public QTextCharFormat
{
public:
    TagTextFormat();
    void setText(const QString &text);
    QString text() const;

protected:
    explicit TagTextFormat(const QTextFormat &fmt);
    friend class InputEdit;
    friend class TagObjectInterface;
};

class InputEdit : public DTK_WIDGET_NAMESPACE::DTextEdit
{
    Q_OBJECT
public:
    explicit InputEdit(QWidget *parent = nullptr);
    QString toConvertedText() const;
    void appendTag(const QString &text);
    void removeTag(const QString &tag);
    bool hasTag(const QString &text);
    void setAutoSelectCode(bool enable) { autoSelectCode = enable; }

signals:
    void pressedEnter();
    void enterReference();
    void tagAdded(const QString &text);
    void tagRemoved(const QString &text);
    void focusOut();

public slots:
    void onTextChanged();

protected:
    bool event(QEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;

private:
    QStringList formatList;
    QMap<QString, TagTextFormat> formats;
    QString selectedCodeTag;
    QString selectedCode;

    bool autoSelectCode = false;
};

class InputEditWidgetPrivate;
class InputEditWidget : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit InputEditWidget(QWidget *parent);
    InputEdit *edit();

    void popupReference();
    void accept(const QModelIndex &index);
    void switchNetworkBtnVisible(bool visible);

signals:
    void pressedEnter();
    void messageSended();
    void handleKey(QKeyEvent *keyEvent);

public slots:
    void onCodeBaseBtnClicked();
    void onReferenceBtnClicked();
    void onNetWorkBtnClicked();
    void onTagAdded(const QString &text);
    void onTagRemoved(const QString &text);

protected:
    bool event(QEvent *e) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    InputEditWidgetPrivate *d;
};

#endif   // INPUTEDITWIDGET_H
