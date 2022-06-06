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
#ifndef SINGLECHOICEBOX_H
#define SINGLECHOICEBOX_H

#include <QGroupBox>
#include <QIcon>

class SingleChoiceBoxPrivate;
class SingleChoiceBox : public QWidget
{
    Q_OBJECT
    SingleChoiceBoxPrivate *const d;
public:
    struct Info {
        QString text;
        QString tooltip;
        QIcon icon;
        inline bool operator == (const Info& info) const {
            return text == info.text
                    && tooltip == info.tooltip;
        }

        inline Info& operator=(const Info& info) {
            text = info.text;
            tooltip = info.tooltip;
            icon = info.icon;
            return *this;
        }
    };

    explicit SingleChoiceBox(QWidget *parent = nullptr);
    virtual ~SingleChoiceBox();
    void setInfos(QSet<Info> infos);
    void setChoiceTitle(const QString &title);

signals:
    void selected(const Info &info);

private slots:
    void toggled(bool checked);
};

uint qHash(const SingleChoiceBox::Info &info, uint seed = 0);

#endif // SINGLECHOICEBOX_H
