// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionmodel.h"

#include "sessionmanager.h"

#include <QFont>
#include <QDateTime>

SessionModel::SessionModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    sessionList = SessionManager::instance()->sessionList();
}

int SessionModel::rowOfSession(const QString &session) const
{
    return sessionList.indexOf(session);
}

QString SessionModel::sessionAt(int row) const
{
    return sessionList.value(row, QString());
}

int SessionModel::rowCount(const QModelIndex &parent) const
{
    return sessionList.size();
}

int SessionModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant SessionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    const auto &sessionName = sessionList.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return sessionName;
        case 1:
            return SessionManager::instance()->sessionDateTime(sessionName);
        }
        break;
    case Qt::FontRole: {
        auto instance = SessionManager::instance();
        QFont font;
        font.setItalic(instance->isDefaultSession(sessionName));
        if (instance->currentSession() == sessionName && !instance->isDefaultVirgin())
            font.setBold(true);
        else
            font.setBold(false);
        return font;
    }
    default:
        break;
    }
    return {};
}

QVariant SessionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            return section == 0 ? tr("Session") : tr("Last Modified");
        }
    }
    return QVariant();
}

void SessionModel::sort(int column, Qt::SortOrder order)
{
    beginResetModel();
    qSort(sessionList.begin(), sessionList.end(),
          [column, order](const QString &s1, const QString &s2) {
              bool ret = false;
              if (column == 0) {
                  ret = s1 < s2;
              } else {
                  const auto &time1 = SessionManager::instance()->sessionDateTime(s1);
                  const auto &time2 = SessionManager::instance()->sessionDateTime(s2);
                  ret = time1 < time2;
              }
              return order == Qt::AscendingOrder ? ret : !ret;
          });

    sortColumn = column;
    sortOrder = order;
    endResetModel();
}

void SessionModel::reset()
{
    beginResetModel();
    sessionList = SessionManager::instance()->sessionList();
    endResetModel();
    sort(sortColumn, sortOrder);
}
