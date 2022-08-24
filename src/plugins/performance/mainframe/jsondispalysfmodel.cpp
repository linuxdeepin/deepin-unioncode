/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#include "jsondispalysfmodel.h"

#include <QRegularExpression>

class JsonDispalySFModelPrivate
{
    friend class JsonDispalySFModel;
    QStringList regExpStrs
    {
        {"(?<float>[0-9]+.[0-9]+)%$"},
        {"(?<llong>[0-9]+)"},
        {"(?<path>[/\\S]+)"}
    };

    void getPathName(const QString &path, QString &perfix, QString &name)
    {
        QString temp = path;
        int endIdx = temp.size() - 1;
        while (endIdx >= 0) {
            if (temp[endIdx] == "/")
                break;
            endIdx --;
        }
        if (endIdx >= 0) {
            name = temp.mid(endIdx + 1, temp.size());
            perfix = temp.mid(0, endIdx);
        }
    }
};

JsonDispalySFModel::JsonDispalySFModel(QObject *parent)
    : QSortFilterProxyModel (parent)
    , d (new JsonDispalySFModelPrivate)
{

}

JsonDispalySFModel::~JsonDispalySFModel()
{
    if (d)
        delete d;
}

bool JsonDispalySFModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString leftStr =  source_left.data(Qt::DisplayRole).toString();
    QString rightStr = source_right.data(Qt::DisplayRole).toString();
    for (auto regExpStr : d->regExpStrs) {
        QRegularExpression regExp(regExpStr);
        auto leftMatchs = regExp.match(leftStr);
        auto rightMatchs = regExp.match(rightStr);
        if (leftMatchs.isValid() && rightMatchs.isValid()) {
            QString left_fMatch = leftMatchs.captured("float");
            QString right_fMatch = rightMatchs.captured("float");
            if (!left_fMatch.isEmpty() && !right_fMatch.isEmpty()) {
                return left_fMatch.toFloat() > right_fMatch.toFloat();
            }
            QString left_llMatch = leftMatchs.captured("llong");
            QString right_llMatch = rightMatchs.captured("llong");
            if (!left_llMatch.isEmpty() && !right_llMatch.isEmpty()) {
                return left_llMatch.toLongLong() > right_llMatch.toLongLong();
            }
            QString left_pMatch = leftMatchs.captured("path").toLower();
            QString right_pMatch = rightMatchs.captured("path").toLower();
            if (!left_pMatch.isEmpty() && !right_pMatch.isEmpty()) {
                return left_pMatch > right_pMatch;
            }
        }
    }
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}
