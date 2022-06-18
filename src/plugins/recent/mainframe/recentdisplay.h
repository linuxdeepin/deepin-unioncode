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
    enum ItemRole{
        ProjectKitName = Qt::ItemDataRole::UserRole,
        ProjectLanguage,
        ProjectWorkspace,
    };
    explicit RecentDisplay(QWidget *parent = nullptr);
    virtual ~RecentDisplay();
    static RecentDisplay *instance();

public slots:
    void addDocument(const QString &filePath);
    void addProject(const QString &filePath,
                    const QString &kitName,
                    const QString &language,
                    const QString &workspace);
};

#endif // RECENTDISPLAY_H
