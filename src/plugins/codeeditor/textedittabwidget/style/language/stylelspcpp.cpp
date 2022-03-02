#include "stylelspcpp.h"

#include "common/common.h"

#include <QDir>
#include <QRegularExpression>

namespace {
static bool checkVersionOk = false;
}

StyleLspCpp::StyleLspCpp()
{

}

StyleLsp::ServerInfo StyleLspCpp::clientInfoSpec(StyleLsp::ServerInfo info)
{
    QString customClangd = QString(LIBRARY_INSTALL_PREFIX) + QDir::separator() + "tools"
            + QDir::separator() + info.progrma;
    QFileInfo customClangdInfo(customClangd);
    qInfo() << "custom clangd path:" << customClangdInfo.filePath();
    qInfo() << "source clangd path:" << info.progrma;

    // 优先使用自定义环境
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
                if (major.toInt() < 10 && !::checkVersionOk) {
                    checkVersionOk = true;
                    QString mess = lsp::Client::tr("clangd lower verion: 10 from %0.\n"
                                                   "Does not meet the current operating environment");
                    mess += QString("\n%0").arg(info.progrma);
                    mess += QString("\n%0").arg(customClangd);
                    ContextDialog::ok(mess);
                }
            }
        }
    }

    return info;
}
