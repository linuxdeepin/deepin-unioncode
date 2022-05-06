#include "projectservice.h"

#include <QTabWidget>

namespace dpfservice {

extern const QString MWCWT_PROJECTS {QTabWidget::tr("Projects")};

dpfservice::ProjectService::ProjectService(QObject *parent)
    : dpf::PluginService (parent)
{

}

QString dpfservice::ProjectService::name()
{
    return "org.deepin.service.ProjectService";
}

}

