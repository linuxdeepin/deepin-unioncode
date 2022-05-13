#ifndef ROUTE_H
#define ROUTE_H

#include "handler.h"

#include <QHash>
#include <QTcpSocket>

class Route
{
public:
    Route();

    struct Head
    {
        QString workspace;
        QString language;
        bool isEmpty() const {
            return workspace.isEmpty()
                    && language.isEmpty();
        }
    };

    static Route *instance();

    Handler *backend(const Head &head) const;
    Handler *frontend(const Head &head) const;

    bool saveFrontend(const Head &head, Handler *front);
    bool saveBackend(const Head &head, Handler *back);
    bool removeFreeFronted(Handler *front);
    bool removeFreeBackend(Handler *back);
    Route::Head findHead(Handler *front = nullptr,
                         Handler *back = nullptr) const;
    Handler *findFrontend(Handler *backend) const;
    Handler *findBackend(Handler *frontend) const;
    Handler *getFrontend(const Head &head) const;
    Handler *getBackend(const Head &head) const;
    Handler *createBackend(const Head &head) const;

private:
    QHash<Head, QPair<Handler *, Handler *>> handlerHash;
};

uint qHash(const Route::Head &key, uint seed = 0);

bool operator == (const Route::Head &t1, const Route::Head &t2);

#endif // ROUTE_H
