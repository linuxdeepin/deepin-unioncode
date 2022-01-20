#ifndef SUPPORTFILE_H
#define SUPPORTFILE_H

#include <QString>

struct SupportFile
{

    struct BuildFileInfo
    {
        QString buildSystem;
        QString projectPath;
        bool operator==(const BuildFileInfo &info) const;
        bool isEmpty();
    };

    struct Builder
    {
        static QString globalPath();
        static QString userPath();
        static void initialize();
        static QString buildSystem(const QString &filePath);
        static BuildFileInfo buildInfo(const QString &filePath);
        // 多线程线程执行
        static QList<BuildFileInfo> buildInfos(const QString &dirPath);
        static bool load();
    };

    struct Language
    {
        static QString globalPath();
        static QString userPath();
    };

    struct EditorStyle
    {
        static QString globalPath(const QString &languageID);
        static QString userPath(const QString &languageID);
    };

    struct WindowStyle
    {
        static QString globalPath();
        static QString userPath();
    };
};

uint qHash(const SupportFile::BuildFileInfo &info, uint seed = 0);
#endif // SUPPORTFILE_H
