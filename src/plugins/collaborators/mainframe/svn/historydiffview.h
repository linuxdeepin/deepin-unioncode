// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HISTORYDIFFVIEW_H
#define HISTORYDIFFVIEW_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QVBoxLayout;
class FileDiffView;
class HistoryDiffView : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryDiffView(const QString &title = "", QWidget *parent = nullptr);
    FileDiffView *getDiffView() const;
    void setTitle(const QString &title);
    QString getTitle() const;

private:
    QVBoxLayout *vLayout{nullptr};
    QLabel *titleLabel{nullptr};
    QLineEdit *searchEdit{nullptr};
    FileDiffView *diffView{nullptr};
};

#endif // HISTORYDIFFVIEW_H
