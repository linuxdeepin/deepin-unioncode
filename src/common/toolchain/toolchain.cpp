#include "toolchain.h"
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

namespace toolchain {
const QString K_VERSION{"version"};
const QString K_HOST_OS{"host_os"};
const QString K_HOST_ARCH{"host_arch"};
const QString K_HOST_KERNEL{"host_kernel"};
const QString K_TOOLCHAINS{"toolchains"};
const QString K_TOOLCHAIN_NAME{"toolchain_name"};
const QString K_TOOLCHAIN_ABI{"toolchain_abi"};
const QString K_TOOLCHAIN_PREFIX{"toolchain_prefix"};
const QString K_TOOLCHAIN_PATH{"toolchain_path"};
const QString K_TOOLCHAIN_C_COMPILER{"toolchain_c_compiler"};
const QString K_TOOLCHAIN_CXX_COMPILER{"toolchain_cxx_compiler"};
const QString K_TOOLCHAIN_DEBUGGER{"toolchain_debugger"};
} // namespace toolchain
