#ifndef PROJECTACTIONINFO_H
#define PROJECTACTIONINFO_H

#include <QString>
#include <QAction>

namespace dpfservice {

template<class T>
struct ParseInfo
{
    T result;
    bool isNormal = true;
};

struct ProjectActionInfo
{
    QString displyText;
    QString tooltip;
    QString buildProgram;
    QStringList buildArguments;
    QString workingDirectory;

    inline bool isInvalid() {
        if (displyText.isEmpty()
                || buildProgram.isEmpty()
                || buildArguments.isEmpty()
                || workingDirectory.isEmpty()
                || workingDirectory.isEmpty())
            return true;
        return false;
    }

    static void set(QAction *action, const ProjectActionInfo &info) {
        action->setProperty("ProjectActionInfo",
                            QVariant::fromValue<ProjectActionInfo>(info));
    }

    static ProjectActionInfo get(QAction *action) {
        if (!action) return {};
        QVariant variant = action->property("ProjectActionInfo");
        if (variant.canConvert<ProjectActionInfo>()) {
            return qvariant_cast<ProjectActionInfo>(variant);
        }
        return {};
    }
};

typedef QList<ProjectActionInfo> ProjectActionInfos;

} // namespace dpfservice

Q_DECLARE_METATYPE(dpfservice::ProjectActionInfo);

#endif // PROJECTACTIONINFO_H
