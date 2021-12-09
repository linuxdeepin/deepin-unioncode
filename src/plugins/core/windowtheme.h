#ifndef WINDOWTHEME_H
#define WINDOWTHEME_H

#include <QString>

class WindowTheme
{
public:
    WindowTheme() = delete;
    static void setTheme(const QString &file);
};

#endif // WINDOWTHEME_H
