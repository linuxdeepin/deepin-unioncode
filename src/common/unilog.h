// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef UNILOG_H
#define UNILOG_H

#include <QDebug>

#ifdef QT_DEBUG
#define UNI_DEBUG
#endif
#ifdef UNI_DEBUG
#define uniDebug(...) qDebug(__VA_ARGS__)
#define uniInfo(...) qInfo(__VA_ARGS__)
#define uniWarning(...) qWarning(__VA_ARGS__)
#define uniCritical(...) qCritical(__VA_ARGS__)
#else    
#define NO_DEBUG(LOG) QMessageLogger().noDebug(LOG)
#define uniDebug(...) NO_DEBUG(__VA_ARGS__)
#define uniInfo(...) NO_DEBUG(__VA_ARGS__)
#define uniWarning(...) NO_DEBUG(__VA_ARGS__)
#define uniCritical(...) NO_DEBUG(__VA_ARGS__)
#endif

#endif // UNILOG_H
