#ifndef MENUEXT_H
#define MENUEXT_H

template <class T, class EnumExt_T>
struct __ext_enum
{
    typedef T type_value;

    static T value(int index) {
        auto baseAddr = (const char*)(EnumExt_T::get());
        return T (*(const T*)(baseAddr + sizeof(T) *index));
    }

    static int index(const T &elem)
    {
        for (int i = 0; i < EnumExt_T::count(); i++) {
            if (value(i) == elem)
                return i;
        }
        return -1;
    }

    static int len() {
        return sizeof(EnumExt_T);
    }

    static int count() {
        return len() == 0 ? 0 : len() / sizeof(type_value);
    }

    static auto get(){
        static EnumExt_T instance;
        return &instance;
    }
};

#define enum_exp public: const type_value
#define enum_def(name, type) class name : public __ext_enum<type, name>
#define enum_type(name) name::type_value

#endif // MENUEXT_H
