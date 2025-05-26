// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYNTAXDEFINITIONNAME_H
#define SYNTAXDEFINITIONNAME_H

#include <QMap>
#include <QString>

namespace ideSyntaxNameMap {
QMap<QString, QString> definitionName = { { "c", "C" },
                                          { "cpp", "C++" },
                                          { "python", "Python" },
                                          { "java", "Java" },
                                          { "javascript", "JavaScript" },
                                          { "go", "Go" },
                                          { "css", "CSS" },
                                          { "php", "PHP" },
                                          { "csharp", "C#" },
                                          { "erlang", "Erlang" },
                                          { "lua", "Lua" } };
}

#endif   // SYNTAXDEFINITIONNAME_H
