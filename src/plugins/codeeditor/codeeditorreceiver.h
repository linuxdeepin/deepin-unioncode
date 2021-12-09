#ifndef CODEEDITORRECEIVER_H
#define CODEEDITORRECEIVER_H

#include <framework/framework.h>

class CodeEditorReceiver: public dpf::EventHandler, dpf::AutoEventHandlerRegister<CodeEditorReceiver>
{
    friend class dpf::AutoEventHandlerRegister<CodeEditorReceiver>;
public:
    explicit CodeEditorReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};


class DpfEventMiddleware : public QObject
{
    Q_OBJECT
    DpfEventMiddleware(){}
    DpfEventMiddleware(const DpfEventMiddleware&) = delete;

public:
    static DpfEventMiddleware& instance();

signals:
    void toOpenFile(const QString &filePath);
};

#endif // CODEEDITORRECEIVER_H
