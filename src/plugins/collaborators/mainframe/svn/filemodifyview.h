// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include "basetype.h"

#include <QTableView>
#include <QFileIconProvider>

class QStandardItem;
class FileListDelegate; // gitqlient class;
class FileModifyViewPrivate;
class FileModifyView : public QTableView
{
    Q_OBJECT
    FileModifyViewPrivate *const d;
signals:
    void diffChecked(const RevisionFile &file);
    void menuRequest(const RevisionFile &file, const QPoint &global);

public:
    explicit FileModifyView(QWidget *parent = nullptr);
    RevisionFile file(int row);
    RevisionFiles files();
    void clean();
    void addFile(const RevisionFile &file);
    void addFiles(const RevisionFiles &files);
    void setFiles(const RevisionFiles &files);
    bool removeFile(const RevisionFile &file);
    int rowCount();
private:
    void showContextMenu(const QPoint &);
    QList<QStandardItem *> createRows(const RevisionFile &file);
    RevisionFile createFile(int row);
};

#endif // FILELISTWIDGET_H
