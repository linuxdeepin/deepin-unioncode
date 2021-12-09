#ifndef SENDEVENTS_H
#define SENDEVENTS_H

#include <QString>

class SendEvents final
{
    SendEvents() = delete;
    SendEvents(const SendEvents &) = delete;
public:
    static void treeViewDoublueClicked(const QString &filePath);
    static void buildProject(const QString &buildSystem,
                             const QString &buildDir,
                             const QString &buildFilePath,
                             const QStringList &buildArgs);
};

#endif // SENDEVENTS_H
