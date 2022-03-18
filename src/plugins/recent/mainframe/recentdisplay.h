#ifndef RECENTDISPLAY_H
#define RECENTDISPLAY_H

#include <QWidget>
#include "common/common.h"

class RecentDisplayPrivate;
class RecentDisplay : public QWidget
{
    Q_OBJECT
    RecentDisplayPrivate *const d;
public:
    explicit RecentDisplay(QWidget *parent = nullptr);
    virtual ~RecentDisplay();
    static RecentDisplay *instance();

public slots:
    void addDocument(const QString &filePath);
    void addFolder(const QString &filePath);

signals:
    void doubleClickedFolder(const QString &filePath);
    void doubleClickedDocument(const QString &filePath);
};

#endif // RECENTDISPLAY_H
