#include "stylekeeper.h"

StyleKeeper::StyleKeeper()
{

}

StyleKeeper *StyleKeeper::instance()
{
    static StyleKeeper ins;
    return &ins;
}

StyleSet StyleKeeper::create(const QString &language, QString *errorString)
{
    auto lspStyle_p = instance()->lspStyleManager.value(language);
    if (!lspStyle_p) {
        lspStyle_p = instance()->lspStyleFactory.create(language);
        instance()->lspStyleManager.append(language, lspStyle_p, errorString);
    }

    auto sciStyle_p = instance()->sciStyleManager.value(language);
    if (!sciStyle_p) {
        sciStyle_p = instance()->sciStyleFactory.create(language);
        instance()->sciStyleManager.append(language, sciStyle_p, errorString);
    }

    return StyleSet { lspStyle_p, sciStyle_p };
}

StyleJsonFile *StyleKeeper::fileJson(const QString &language, QString *errorString)
{
    auto fileStyle_p = instance()->docJsonManager.value(language);
    if (!fileStyle_p) {
        fileStyle_p = instance()->docJsonFactory.create(language);
        if (!fileStyle_p) {
            qCritical() << "Failed, create json file from: " << language;
            abort();
        }
        fileStyle_p->setLanguage(language);
        instance()->docJsonManager.append(language, fileStyle_p, errorString);
    }
    return fileStyle_p;
}

QString StyleKeeper::key( StyleLsp *lsp)
{
    return instance()->lspStyleManager.key(lsp);
}

QString StyleKeeper::key(StyleSci *sci)
{
    return instance()->sciStyleManager.key(sci);
}

QString StyleKeeper::key(StyleJsonFile *file)
{
    return instance()->docJsonManager.key(file);
}

StyleSet& StyleSet::operator = (const StyleSet &styleSet)
{
    lsp = styleSet.lsp;
    sci = styleSet.sci;
    return *this;
}
