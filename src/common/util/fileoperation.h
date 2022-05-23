#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QString>

class FileOperation final
{
public:
    static bool doMoveMoveToTrash(const QString &filePath);
    static bool doRecoverFromTrash(const QString &filePath);
    static bool doRemove(const QString &filePath);
    static bool doNewDocument(const QString &parentPath, const QString &docName);
    static bool doNewFolder(const QString &parentPath, const QString &folderName);
};

#endif // FILEOPERATION_H
