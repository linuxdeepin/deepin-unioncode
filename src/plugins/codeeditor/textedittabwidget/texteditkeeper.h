#ifndef TEXTEDITKEEPER_H
#define TEXTEDITKEEPER_H

#include "textedit.h"
#include <framework/service/qtclassfactory.h>
#include <framework/service/qtclassmanager.h>

class TextEditKeeper final
{
    dpf::QtClassFactory<TextEdit> editFactory;

    TextEditKeeper(){}

    inline static TextEditKeeper *instance(){
        static TextEditKeeper ins;
        return &ins;
    }

public:

    template<class Edit>
    static bool impl(const QString &language, QString *err = nullptr) {
        bool result = true;
        result &= instance()->editFactory.regClass<Edit>(language, err);
        return result;
    }

    static TextEdit *create(const QString &language, QString *err = nullptr)
    {
        return instance()->editFactory.create(language, err);
    }
};

#endif // TEXTEDITKEEPER_H
