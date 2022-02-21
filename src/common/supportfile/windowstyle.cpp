#include "windowstyle.h"
#include "util/custompaths.h"

#include <QJsonDocument>

namespace support_file {

namespace documents {

inline static QJsonDocument windowStyleGlobal;
inline static QJsonDocument windowStyleUser;
bool windowStyleIsLoaded();

} // namespace documents

bool documents::windowStyleIsLoaded()
{
    return !windowStyleGlobal.isEmpty() && !windowStyleGlobal.isEmpty();
}

QString WindowStyle::globalPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Configures));
    return result + QString("windowstyle.support");
}

QString WindowStyle::userPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::user(CustomPaths::Configures));
    return result + QString("windowstyle.support");
}

} // namespace support_file
