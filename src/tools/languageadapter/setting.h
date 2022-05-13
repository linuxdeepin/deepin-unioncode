#ifndef SETTING_H
#define SETTING_H

#include "settinginfo.h"

class Setting
{
    Setting() = delete;
    Setting(const Setting &) = delete;
public:
    static SettingInfo getInfo(const QString &language);
    static bool genConfigFile(const QString &configPath);
};

#endif // SETTING_H
