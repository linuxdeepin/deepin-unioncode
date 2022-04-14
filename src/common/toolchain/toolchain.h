#ifndef TOOLCHAIN_H
#define TOOLCHAIN_H
#include <QString>

namespace toolchains {
namespace cxx {
extern const QString K_VERSION;
extern const QString K_HOST_OS;
extern const QString K_HOST_ARCH;
extern const QString K_HOST_KERNEL;
extern const QString K_TOOLCHAINS;
extern const QString K_TOOLCHAIN_NAME;
extern const QString K_TOOLCHAIN_ABI;
extern const QString K_TOOLCHAIN_PREFIX;
extern const QString K_TOOLCHAIN_PATH;
extern const QString K_TOOLCHAIN_C_COMPILER;
extern const QString K_TOOLCHAIN_CXX_COMPILER;
extern const QString K_TOOLCHAIN_DEBUGGER;

bool generatGlobalFile();

} //cxx

} // namespace toolchains

#endif
