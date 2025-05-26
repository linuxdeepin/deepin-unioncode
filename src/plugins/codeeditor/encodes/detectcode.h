// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETECTCODE_H
#define DETECTCODE_H

#include <QString>
#include <QMap>

#include <chardet/chardet.h>
#include <uchardet/uchardet.h>
#include <iconv.h>

#include <unicode/ucnv.h>
#include <unicode/utypes.h>
#include <unicode/ucsdet.h>
#include <unicode/umachine.h>
#include <unicode/urename.h>

/*
 *
 * Text encoding detection using third-party libraries chardet1 and uchardet
 * If chardet fails to detect, uchardet will be used
 * Using iconv library for encoding conversion
 *
 */

class QByteArray;
class QString;

class DetectCode
{
public:
    // libchardet1 encoding detection library
    static int chartDetDetectingTextCoding(const char *str, QString &encoding, float &confidence);
    // Detect text encoding using uchardet
    static QByteArray uchardetCode(QString filePath);
    // Detect encoding using ICU library
    static void icuDetectTextEncoding(const QString &filePath, QByteArrayList &listDetectRet);
    // Inner function for ICU encoding detection
    static bool detectTextEncoding(const char *data, size_t len, char **detected, QByteArrayList &listDetectRet);
    // Filter detected encodings
    static QByteArray selectCoding(QByteArray ucharDetectRet, QByteArrayList icuDetectRetList, float confidence);
    // Get file encoding format
    static QByteArray getFileEncodingFormat(QString filePath, QByteArray content = QByteArray(""));
    // Convert text encoding format
    static bool changeFileEncodingFormat(QByteArray &inputStr,
                                         QByteArray &outStr,
                                         const QString &fromCode,
                                         const QString &toCode = QString("UTF-8"));
    static bool convertEncodingTextCodec(QByteArray &inputStr,
                                         QByteArray &outStr,
                                         const QString &fromCode,
                                         const QString &toCode = QString("UTF-8"));

private:
    static QMap<QString, QByteArray> sm_LangsMap;
};

#endif   // DETECTCODE_H
