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
    StyleSet setting;
    auto lspStyle_p = instance()->lspStyleManager.value(language);
    if (!lspStyle_p) {
        lspStyle_p = instance()->lspStyleFactory.create(language);
        instance()->lspStyleManager.append(language, lspStyle_p, errorString);
    }
    setting.lsp = lspStyle_p;

    auto sciStyle_p = instance()->sciStyleManager.value(language);
    if (!sciStyle_p) {
        sciStyle_p = instance()->sciStyleFactory.create(language);
        instance()->sciStyleManager.append(language, sciStyle_p, errorString);
    }
    setting.sci = sciStyle_p;

    auto fileStyle_p = instance()->docJsonFactory.create(language);
    if (!fileStyle_p) {
        fileStyle_p = instance()->docJsonFactory.create(language);
        if (!fileStyle_p) {
            qCritical() << "Failed, create json file from: " << language;
            abort();
        }
        fileStyle_p->setLanguage(language);
        instance()->docJsonManager.append(language, fileStyle_p, errorString);
    }
    setting.file = fileStyle_p;

    return setting;
}

bool StyleSet::isEmpty()
{
    return lsp == nullptr || sci == nullptr || file == nullptr;
}

bool StyleSet::operator == (const StyleSet &styleSet)
{
    return lsp == styleSet.lsp
            && sci == styleSet.sci
            && file == styleSet.file;
}

StyleSet& StyleSet::operator = (const StyleSet &styleSet)
{
    lsp = styleSet.lsp;
    sci = styleSet.sci;
    file = styleSet.file;
    return *this;
}
