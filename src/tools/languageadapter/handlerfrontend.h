#ifndef HANDLERFRONTEND_H
#define HANDLERFRONTEND_H

#include "handler.h"

class HandlerFrontend : public Handler
{
public:
    HandlerFrontend();
    void filterData(const QByteArray &array);
    virtual void request(const QByteArray &array);
    virtual bool initRequest(const QJsonObject &obj);
};

#endif // HANDLERFRONTEND_H
