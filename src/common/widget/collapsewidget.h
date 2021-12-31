/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#ifndef COLLAPSEWIDGET_H
#define COLLAPSEWIDGET_H

#include <QPaintEvent>
#include <QWidget>

class CollapseWidgetPrivate;
class CollapseWidget : public QWidget
{
    CollapseWidgetPrivate *const d;
public:
    explicit CollapseWidget(QWidget *parent = nullptr);
    explicit CollapseWidget(const QString &title, QWidget *widget, QWidget *parent = nullptr);
    virtual ~CollapseWidget() override;

    QWidget *takeWidget();
    void setWidget(QWidget *widget);
    QWidget* widget();

    void setTitle(const QString &title);
    QString title();

    void setCheckable(bool checkable);
    bool isCheckable();

    bool isChecked();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

public slots:
    void setChecked(bool);

private slots:
    void doChecked(bool);
    void resetHeight(const QVariant &);
};

#endif // COLLAPSEWIDGET_H
