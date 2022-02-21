#ifndef WINDOWSTYLE_H
#define WINDOWSTYLE_H

#include <QString>
namespace support_file {

struct WindowStyle
{
    static QString globalPath();
    static QString userPath();
};

}
#endif // WINDOWSTYLE_H
