#ifndef HANDLERBACKEND_H
#define HANDLERBACKEND_H

#include "handler.h"
#include "settinginfo.h"

class HandlerBackend : public Handler
{
    Q_OBJECT
public:

    HandlerBackend(const SettingInfo &info);

    virtual void bind(QProcess *qIODevice);
    virtual void filterData(const QByteArray &array);
    virtual void response(const QByteArray &array);
    virtual bool exitNotification(const QJsonObject &obj);

    virtual SettingInfo getSettingInfo() {return info;}

private slots:
    void doFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    SettingInfo info;
};

#endif // HANDLERBACKEND_H
