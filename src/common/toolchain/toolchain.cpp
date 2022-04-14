#include "toolchain.h"
#include "util/processutil.h"
#include "util/custompaths.h"

#include <QFile>
#include <QDir>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

namespace toolchain {
namespace cxx {
const QString K_SCIRPTNAME{"toolchain.sh"};
const QString K_FILENAME{"toolchains_cxx.support"};
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
} // namespace cxx
} // namespace toolchain

bool toolchains::cxx::generatGlobalFile()
{
    using namespace toolchain::cxx;
    // 获取已安装的工具链扫描脚本
    auto script = CustomPaths::global(CustomPaths::Scripts) + QDir::separator() + K_SCIRPTNAME;
    // 执行扫描脚本，隐式规则 "$HOME/.config/unioncode/configures/toolchains.support"
    ProcessUtil::execute(script, {}, [=](const QByteArray &out){
        qInfo() << out;
    });
    // 返回脚本标定路径
    QString result = CustomPaths::user(CustomPaths::Configures) + QDir::separator() + K_FILENAME;
    if (QFile(result).exists())
        return true;
    else
        return false;
}

