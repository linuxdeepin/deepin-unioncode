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
    static QString global(Flage flage);
    static QString user(Flage flage);
};

#endif // GLOBALCONFIGURE_H
