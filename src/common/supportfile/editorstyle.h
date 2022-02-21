#ifndef EDITORSTYLE_H
#define EDITORSTYLE_H

#include <QString>

namespace support_file {

struct EditorStyle
{
    static QString globalPath(const QString &languageID);
    static QString userPath(const QString &languageID);
    static void initialize(const QString &languageID);
    static bool recovery(const QString &languageID);
};

}

#endif // EDITORSTYLE_H
