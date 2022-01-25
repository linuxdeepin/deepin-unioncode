#ifndef STYLEKEEPER_H
#define STYLEKEEPER_H

#include "stylelsp.h"
#include "stylesci.h"
#include "stylejsonfile.h"

#include <framework/service/qtclassfactory.h>
#include <framework/service/qtclassmanager.h>

struct StyleSet
{
    const StyleLsp *lsp;
    const StyleSci *sci;
    const StyleJsonFile *file;
    bool isEmpty();
    bool operator == (const StyleSet &styleSet);
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

    template<class T_Lsp, class T_Sci, class T_File>
    static bool regClass(const QString &language, QString *errorString = nullptr) {
        bool result = true;
        result &= instance()->lspStyleFactory.regClass<T_Lsp>(language, errorString);
        result &= instance()->sciStyleFactory.regClass<T_Sci>(language, errorString);
        result &= instance()->docJsonFactory.regClass<T_File>(language, errorString);
        return result;
    }

    static StyleSet create(const QString &language, QString *errorString);
};

#endif // STYLEKEEPER_H
