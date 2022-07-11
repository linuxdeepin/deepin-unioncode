#ifndef BACKENDCHECKER_H
#define BACKENDCHECKER_H

#include <QWidget>
#include <QUrl>
#include <QProcess>
#include <QProgressBar>
#include <QDir>
#include <QTextBrowser>
#include <QCryptographicHash>

class RequestInfo
{
    QUrl packageUrl;
    QString packageSaveName;
    QUrl checkFileUrl;
    QString checkFileSaveName;
    QString checkNumProgram;
    QString checkNumMode;
public:
    RequestInfo() = default;
    RequestInfo(const QUrl &packageUrl,
                const QString &packageSaveName,
                const QUrl &checkFileUrl,
                const QString &checkFileSaveName,
                const QString &checkNumProgram,
                const QString &checkNumMode)
        : packageUrl(packageUrl),
          packageSaveName(packageSaveName),
          checkFileUrl(checkFileUrl),
          checkFileSaveName(checkFileSaveName),
          checkNumProgram(checkNumProgram),
          checkNumMode(checkNumMode){}

    QUrl getPackageUrl() const;
    QString getPackageSaveName() const;
    QUrl getCheckFileUrl() const;
    QString getCheckFileSaveName() const;
    QString getCheckNumProgram() const;
    QString getCheckNumMode() const;

    void setPackageUrl(const QUrl &value);
    void setPackageSaveName(const QString &value);
    void setCheckFileUrl(const QUrl &value);
    void setCheckFileSaveName(const QString &value);
    void setCheckNumProgram(const QString &value);
    void setCheckNumMode(const QString &value);
};

class BackendChecker : public QWidget
{
    Q_OBJECT
public:
    explicit BackendChecker(QWidget *parent = nullptr);
    bool exists(const QString &languageID);
    bool checkCachePackage(const QString &languageID);
private:
    QString adapterPath;
    QHash<QString, RequestInfo> requestInfos;
};

#endif // BACKENDCHECKER_H
