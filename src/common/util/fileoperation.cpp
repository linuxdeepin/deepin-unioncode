#include "fileoperation.h"
#include "processutil.h"
#include "common.h"

#include <QDialog>
#include <QDir>
#include <QFileInfo>

const QString DELETE_MESSAGE_TEXT {QDialog::tr("The delete operation will be removed from"
                                               "the disk and will not be recoverable "
                                               "after this operation.\nDelete anyway?")};

const QString DELETE_WINDOW_TEXT {QDialog::tr("Delete Warning")};

bool FileOperation::doMoveMoveToTrash(const QString &filePath)
{
    return ProcessUtil::moveToTrash(filePath);
}

bool FileOperation::doRecoverFromTrash(const QString &filePath)
{
    return ProcessUtil::recoverFromTrash(filePath);
}

bool FileOperation::doRemove(const QString &filePath)
{
    return QFile(filePath).remove();
}

bool FileOperation::doNewDocument(const QString &parentPath, const QString &docName)
{
    QFileInfo info(parentPath);
    if (!info.exists() || !info.isDir())
        return false;
    else  {
        QFile file(parentPath + QDir::separator() + docName);
        if (file.open(QFile::OpenModeFlag::NewOnly)){
            file.close();
        }
        return false;
    }
}

bool FileOperation::doNewFolder(const QString &parentPath, const QString &folderName)
{
    QFileInfo info(parentPath);
    if (!info.exists() || !info.isDir())
        return false;
    else
        return QDir(parentPath).mkdir(folderName);
}
