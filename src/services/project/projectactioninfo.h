// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTACTIONINFO_H
#define PROJECTACTIONINFO_H

#include <QString>
#include <QAction>

namespace dpfservice {

struct ProjectMenuActionInfo
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

    static void set(QAction *action, const ProjectMenuActionInfo &info) {
        action->setProperty("ProjectActionInfo",
                            QVariant::fromValue<ProjectMenuActionInfo>(info));
    }

    static ProjectMenuActionInfo get(QAction *action) {
        if (!action) return {};
        QVariant variant = action->property("ProjectActionInfo");
        if (variant.canConvert<ProjectMenuActionInfo>()) {
            return qvariant_cast<ProjectMenuActionInfo>(variant);
        }
        return {};
    }
};

typedef QList<ProjectMenuActionInfo> ProjectActionInfos;

} // namespace dpfservice

Q_DECLARE_METATYPE(dpfservice::ProjectMenuActionInfo);

#endif // PROJECTACTIONINFO_H
