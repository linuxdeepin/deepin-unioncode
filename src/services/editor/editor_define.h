// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITOR_DEFINE_H
#define EDITOR_DEFINE_H

namespace dpfservice {
namespace Edit {

enum AnnotationType {
    NoteAnnotation = 0,
    WarningAnnotation,
    ErrorAnnotation,
    FatalAnnotation
};

struct Position
{
    int line = -1;
    int column = -1;
};

struct Range
{
    Position start;
    Position end;
};

}
}

#endif   // EDITOR_DEFINE_H
