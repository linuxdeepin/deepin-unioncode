// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSDEFINE_H
#define SETTINGSDEFINE_H

#include <QString>
#include <QStringList>

constexpr char EditorConfig[] { "Editor" };

namespace Node {
constexpr char FontColor[] { "Font & Colors" };
constexpr char Behavior[] { "Behavior" };
constexpr char MimeTypeConfig[] { "MimeType & Comments" };
}

namespace Group {
constexpr char FontGroup[] { "Font" };
constexpr char TabGroup[] { "Tabs And Indentation" };
}

namespace Key {
constexpr char FontSize[] { "fontSize" };
constexpr char FontFamily[] { "fontFamily" };
constexpr char FontZoom[] { "fontZoom" };

constexpr char TabPolicy[] { "tabPolicy" };
constexpr char TabSize[] { "tabSize" };
constexpr char EnableAutoIndentation[] { "enableAutoIndentation" };

constexpr char MimeTypeGroupName[] { "groupName" };
constexpr char MimeType[] { "mimeType" };
constexpr char SingleLineComment[] { "singleLineComment" };
constexpr char StartMultiLineComment[] { "startMultiLineComment" };
constexpr char EndMultiLineComment[] { "endMultiLineComment" };
}

#endif   // SETTINGSDEFINE_H
