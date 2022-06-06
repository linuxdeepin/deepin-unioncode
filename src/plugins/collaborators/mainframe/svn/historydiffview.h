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
