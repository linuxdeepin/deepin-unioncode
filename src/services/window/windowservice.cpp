#include "windowservice.h"
namespace dpfservice {

WindowService::WindowService(QObject *parent)
    : dpf::PluginService (parent)
{

}

QString WindowService::name()
{
    return "org.deepin.service.WindowService";
}

} // namespace dpfservice
