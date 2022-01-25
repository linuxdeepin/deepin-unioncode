#ifndef MENUEXT_H
#define MENUEXT_H

template <class T>struct EnumExt{T _declType;};
#define E public: const decltype(EnumExt::_declType)
#define enum_def(name, type) class name : public EnumExt<type>
#define enum_type(name) decltype(EnumExt::_declType)

#endif // MENUEXT_H
