// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLANGPARSER_H
#define CLANGPARSER_H

#include <QString>

namespace ClangParser {
bool parse(const QString &workSpace, const QString &symbolLocation, const QString &language);
bool parseSingleFile(const QString &filePath, const QString &symbolLocation);
}
#endif // CLANGPARSER_H
