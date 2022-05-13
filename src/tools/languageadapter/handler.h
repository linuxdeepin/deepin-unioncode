#ifndef HANDLER_H
#define HANDLER_H

#include <QObject>
#include <QProcess>
#include <QTcpSocket>

class Handler : public QObject
{
    Q_OBJECT
public:
    Handler();
    virtual ~Handler();
    virtual void bind(QProcess *qIODevice);
    virtual void bind(QTcpSocket *qIODevice);
    virtual void filterData(const QByteArray &array) {Q_UNUSED(array)}
    QIODevice *getDevice() { return device;}

signals:
    void nowReadedAll(const QByteArray &array);

public slots:
    void nowToWrite(const QByteArray &array);

private slots:
    void doReadAll();

protected:
    QIODevice *device;
};

#endif // HANDLER_H
