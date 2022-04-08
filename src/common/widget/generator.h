#ifndef GENERATOR_H
#define GENERATOR_H

#include <QObject>
#include <QVariant>

class Generator : public QObject
{
    Q_OBJECT
public:
    struct Procedure
    {
        const QString &message;
        const int current;
        int max = 100;
    };
    explicit Generator(QObject *parent = nullptr);
    QString errorString();

signals:
    void message(const Procedure &procedure);
    void started();
    void finished();

protected:
    bool setErrorString(const QString &error); //子類調用
    bool setProperty(const QString &name, const QVariant &value); //子類調用
    QVariant property(const QString &name) const; //子類調用
};

#endif // GENERATOR_H
