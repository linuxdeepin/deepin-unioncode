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
#ifndef HISTORYDIFFWIDGET_H
#define HISTORYDIFFWIDGET_H

#include <QSplitter>

class HistoryDiffView;
class HistoryDiffWidget : public QSplitter
{
    Q_OBJECT
public:
    explicit HistoryDiffWidget(QWidget *parent = nullptr);
    HistoryDiffView *getOldView() const;
    HistoryDiffView *getNewView() const;

private:
    HistoryDiffView *oldView{nullptr};
    HistoryDiffView *newView{nullptr};
};

#endif // HISTORYDIFFWIDGET_H
