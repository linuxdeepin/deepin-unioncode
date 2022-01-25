#include "stylelsp.h"
#include "Scintilla.h"

StyleLsp::StyleLsp()
{

}

StyleLsp::~StyleLsp()
{

}

QStringList StyleLsp::themes() const {
    return {};
}

bool StyleLsp::selectTheme(const QString &theme){
    if (!themes().contains(theme)) {
        qCritical() << "Failed, select theme is empty: " << theme
                    << "support themes: " << themes();
        return false;
    }
    return true;
}

int StyleLsp::fontSize(const QVariant &key) const
{
    return 0x000000;
}

int StyleLsp::background(const QVariant &key) const
{
    return 0x000000;
}

int StyleLsp::foreground(const QVariant &key) const
{
    return 0x000000;
}

StyleLsp::Underline StyleLsp::underline(const QVariant &key) const
{
    return {0x000000, INDIC_HIDDEN};
}
