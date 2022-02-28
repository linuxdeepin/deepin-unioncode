#include "stylelspcpp.h"

#include "common/common.h"

#include <QDir>
#include <QRegularExpression>

namespace  {
bool checkVersionOk = false;
}

StyleLspCpp::StyleLspCpp()
{

}

StyleLsp::ServerInfo StyleLspCpp::clientInfoSpec(StyleLsp::ServerInfo info)
{
    // exists language server
    if (info.progrma.isEmpty()) {
        return info;
    }

    QString customClangd = QString(LIBRARY_INSTALL_PREFIX) + QDir::separator() + "tools"
            + QDir::separator() + info.progrma;
    QFileInfo customClangdInfo(customClangd);

    if (customClangdInfo.exists() && customClangdInfo.isExecutable()) {
        return { customClangd, info.arguments };
    }

    if (ProcessUtil::exists(info.progrma)) {
        QRegularExpression regExp("[0-9]*\\.[0-9]*\\.[0-9]*");
        auto versionMatchs = regExp.match(ProcessUtil::version(info.progrma)).capturedTexts();
        for (auto versionMatch : versionMatchs) {
            QStringList versions = versionMatch.split(".");
            if (versions.size() == 3) {
                auto major =  versions[0];
                if (major.toInt() >= 10) { // 版本需要大于等于10
                    ::checkVersionOk = true;
                }
            }
        }
    }

    if (!::checkVersionOk) {
        ContextDialog::ok(lsp::Client::tr("clangd lower verion: 10, "
                                          "Does not meet the current operating environment"));
        return {};
    }

    return info;
}
