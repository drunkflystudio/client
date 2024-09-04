#include "ProjectSelectorWidget.h"
#include "Network/Server.h"

ProjectSelectorWidget::ProjectSelectorWidget(QWidget* parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);
}

ProjectSelectorWidget::~ProjectSelectorWidget()
{
}

void ProjectSelectorWidget::setServer(Server* server)
{
    m_server = server;
}

void ProjectSelectorWidget::on_createButton_triggered()
{
    // FIXME
}
