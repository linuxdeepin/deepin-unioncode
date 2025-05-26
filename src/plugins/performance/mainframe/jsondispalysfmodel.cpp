// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
