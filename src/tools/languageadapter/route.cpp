#include "route.h"
#include "setting.h"
#include "handlerfrontend.h"
#include "handlerbackend.h"

#include <iostream>

Route::Route()
    : handlerHash({})
{

}

Route *Route::instance(){
    static Route ins;
    return &ins;
}

bool Route::saveFrontend(const Route::Head &head, Handler *front)
{
    auto value = handlerHash.value(head);
    if (value.first) {
        std::cerr << __FUNCTION__ << " Current head saved frontend" << "\n"
                  << " workspace: " << head.workspace.toStdString() << "\n"
                  << " language: " << head.language.toStdString() << "\n"
                  << std::endl;
        return false;
    }

    handlerHash.insert(head, {front, value.second});
    return true;
}

bool Route::saveBackend(const Route::Head &head, Handler *back)
{
    auto value = handlerHash.value(head);
    if (value.second) {
        std::cerr << __FUNCTION__ << " Current head saved backend" << "\n"
                  << " workspace: " << head.workspace.toStdString() << "\n"
                  << " language: " << head.language.toStdString() << "\n"
                  << std::endl;
        return false;
    }
    handlerHash.insert(head, {value.first, back});
    return true;
}

bool Route::removeFreeFronted(Handler *front)
{
    auto itera = handlerHash.begin();
    while (itera != handlerHash.end()) {
        auto value = itera.value();
        if (value.first == front) {
            std::cout << __FUNCTION__
                      << " workspace: "
                      << itera.key().workspace.toStdString()
                      << " language: "
                      << itera.key().language.toStdString()
                      << std::endl;
            if (itera->second == nullptr) {
                itera = handlerHash.erase(itera);
            } else {
                delete itera.value().first;
                itera.value() = {nullptr, value.second};
            }
            return true;
        }
        itera ++;
    }
    return false;
}

bool Route::removeFreeBackend(Handler *back)
{
    auto itera = handlerHash.begin();
    while (itera != handlerHash.end()) {
        auto value = itera.value();
        if (value.second == back) {
            std::cout << __FUNCTION__
                      << " workspace:"
                      << itera.key().workspace.toStdString()
                      << " language: "
                      << itera.key().language.toStdString()
                      << std::endl;
            if (itera->first == nullptr) {
                itera = handlerHash.erase(itera);
            } else {
                delete itera.value().second;
                itera.value() = {value.first, nullptr};
            }
            return true;
        }
        itera ++;
    }
    return false;
}



Route::Head Route::findHead(Handler *front, Handler *back) const
{
    if (front && back) {
        return handlerHash.key({front, back});
    } else {
        auto itera = handlerHash.begin();
        while(itera != handlerHash.end()) {
            if (front && front == itera.value().first) {
                return itera.key();
            }
            if (back && back == itera.value().second) {
                return itera.key();
            }
            ++ itera;
        }
    }
    return {};
}

Handler *Route::findFrontend(Handler *backend) const
{
    auto itera = handlerHash.begin();
    while (itera != handlerHash.end()) {
        if (itera.value().second == backend)
            return itera.value().first;
        itera ++;
    }
    return nullptr;
}

Handler *Route::findBackend(Handler *frontend) const
{
    auto itera = handlerHash.begin();
    while (itera != handlerHash.end()) {
        if (itera.value().first == frontend)
            return itera.value().second;
        itera ++;
    }
    return nullptr;
}

Handler *Route::getFrontend(const Route::Head &head) const
{
    return handlerHash.value(head).first;
}

Handler *Route::getBackend(const Route::Head &head) const
{
    return handlerHash.value(head).second;
}

Handler *Route::createBackend(const Route::Head &head) const
{
    auto handler = new HandlerBackend(Setting::getInfo(head.language));
    return handler;
}

uint qHash(const Route::Head &key, uint seed) {
    return qHash(key.workspace + key.language, seed);
}

bool operator ==(const Route::Head &t1, const Route::Head &t2)
{
    return t1.language == t2.language
            && t1.workspace == t2.workspace;
}
