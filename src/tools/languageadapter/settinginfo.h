#ifndef SETTINGINFO_H
#define SETTINGINFO_H

#include <QStringList>
#include <QDebug>

struct SettingInfo
{
    QString language;
    QString mode;
    QString program;
    QStringList arguments;
};

inline QDebug operator << (QDebug debug, const SettingInfo &info) {
    debug << "SettingInfo("
          << info.language
          << info.mode
          << info.program
          << info.arguments;
    return debug;
}
#endif // SETTINGINFO_H
