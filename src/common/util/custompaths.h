#ifndef GLOBALCONFIGURE_H
#define GLOBALCONFIGURE_H

#include <QString>

class CustomPaths final
{
    CustomPaths() = delete;
    Q_DISABLE_COPY(CustomPaths)
public:
    enum Flage{
        Applition,
        DependLibs,
        Plugins,
        Tools,
        Extensions,
        Sources,
        Configures,
        Scripts,
        Translations,
        Templates
    };
    static QString user(Flage flage);
    static bool installed();
};

#endif // GLOBALCONFIGURE_H
