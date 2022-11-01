#pragma once

#include <QtCore/QtGlobal>

#if defined(__GNUC__)
#   define QPinnableTabWidgetLIB_EXPORT __attribute__((visibility("default")))
#else
#   if defined(QPinnableTabWidgetLIB_LIBRARY)
#      define QPinnableTabWidgetLIB_EXPORT Q_DECL_EXPORT
#   else
#      define QPinnableTabWidgetLIB_EXPORT Q_DECL_IMPORT
#   endif
#endif
