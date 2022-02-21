#ifndef STYLEKEEPER_H
#define STYLEKEEPER_H

#include "stylelsp.h"
#include "stylesci.h"
#include "stylejsonfile.h"

#include <framework/service/qtclassfactory.h>
#include <framework/service/qtclassmanager.h>

struct StyleSet
{
    StyleLsp *lsp;
    StyleSci *sci;
    StyleSet& operator = (const StyleSet &styleSet);
};

class StyleKeeper
{
    dpf::QtClassFactory<StyleLsp> lspStyleFactory;
    dpf::QtClassManager<StyleLsp> lspStyleManager;
    dpf::QtClassFactory<StyleSci> sciStyleFactory;
    dpf::QtClassManager<StyleSci> sciStyleManager;
    dpf::QtClassFactory<StyleJsonFile> docJsonFactory;
    dpf::QtClassManager<StyleJsonFile> docJsonManager;
    StyleKeeper();
    static StyleKeeper *instance();

public:

    template<class T_Sci, class T_Lsp>
    static bool regClass(const QString &language, QString *errorString = nullptr) {
        bool result = true;
        result &= instance()->lspStyleFactory.regClass<T_Lsp>(language, errorString);
        result &= instance()->sciStyleFactory.regClass<T_Sci>(language, errorString);
        result &= instance()->docJsonFactory.regClass<StyleJsonFile>(language, errorString);
        return result;
    }
    static StyleSet create(const QString &language, QString *errorString = nullptr);
    static StyleJsonFile *fileJson(const QString &language, QString *errorString = nullptr);
    static QString key(StyleLsp *lsp);
    static QString key(StyleSci *sci);
    static QString key(StyleJsonFile *file);
};

#endif // STYLEKEEPER_H
