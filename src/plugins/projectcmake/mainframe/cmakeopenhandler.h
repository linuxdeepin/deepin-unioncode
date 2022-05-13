#ifndef PROJECTCMAKEOPEN_H
#define PROJECTCMAKEOPEN_H

#include <QObject>
#include <QAction>

class CMakeOpenHandler : public QObject
{
    Q_OBJECT
public:
    explicit CMakeOpenHandler(QObject *parent = nullptr);
    static CMakeOpenHandler *instance();
    QAction *openAction();

signals:
    void projectOpened(const QString &generatorName, const QString &language, const QString &filePath);

public slots:
    void open();
};

#endif // PROJECTCMAKEOPEN_H
